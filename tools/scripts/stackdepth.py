"""Stack bookkeeping of MPW C code, and the game compiler's rule for popping call arguments before 'unlk'.

MPW C leaves call arguments on the stack and pops them later (or never: 'unlk' resets the stack anyway).
C 3.2 and later never pop right before the epilogue. The game's compiler sometimes does. Measured on the
game (122 of 138 functions with arguments left over follow it), it pops when the leftover pushes come
  - after a loop (a backward branch), or
  - after a forward branch, and more is left than the last call's own arguments or code follows that call.
Usage: python -I stackdepth.py <module.mod>   (checks the rule against every game function)
"""
import collections
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gamefuncs import disasm, functions  # noqa: E402

SAVE_REGS = re.compile(r"(movem\.l [ad]\d.*|move\.l [ad]\d), -\(a7\)$")


def push_size(i):
    """Bytes an instruction pushes onto the stack (0 if none)."""
    if i.mnemonic.startswith("pea"):              # Capstone doesn't print pea's implied -(a7)
        return 4
    if not i.op_str.endswith("-(a7)") or i.mnemonic.startswith("movem"):
        return 0
    return {"b": 2, "w": 2, "l": 4}.get(i.mnemonic.rsplit(".", 1)[-1], 4)


def pop_size(i):
    """Bytes an instruction pops (addq/adda/lea on a7), 0 if none."""
    s = f"{i.mnemonic} {i.op_str}"
    m = re.match(r"(?:addq|adda)\.[wl] #\$?([0-9a-f]+), a7$", s)
    if m:
        return int(m.group(1), 16)
    m = re.match(r"lea(?:\.l)? \$?([0-9a-f]+)\(a7\), a7$", s)
    if m:
        return int(m.group(1), 16)
    return 0


def is_branch(i):
    return i.mnemonic.startswith(("b", "db")) and not i.mnemonic.startswith(("btst", "bset", "bclr", "bchg"))


def branch_target(i):
    return int(i.op_str.split("$")[-1], 16) if is_branch(i) and "$" in i.op_str else None


def epilogue(ins):
    """Index where the final epilogue starts: the register restore before the last unlk, else the unlk."""
    k = max(n for n, i in enumerate(ins) if i.mnemonic == "unlk")
    if k > 0 and ins[k - 1].mnemonic.startswith(("movem", "move")) and ins[k - 1].op_str.startswith("-$") \
            and "(a6)" in ins[k - 1].op_str:
        k -= 1
    return k


def analyse(ins):
    """Return (bytes left on the stack at the epilogue, whether the game's compiler would pop them,
    whether the code already pops right before the epilogue, epilogue index)."""
    end = epilogue(ins)
    popped = end > 0 and pop_size(ins[end - 1]) > 0
    stop = end - 1 if popped else end
    start = 2 if len(ins) > 1 and SAVE_REGS.match(f"{ins[1].mnemonic} {ins[1].op_str}") else 1
    pend, calls = [], []
    for k in range(start, stop):
        i = ins[k]
        if push_size(i):
            pend.append((push_size(i), k))
        p = pop_size(i)
        while p > 0 and pend:
            p -= pend.pop()[0]
        if i.mnemonic == "jsr":
            calls.append(k)
    left = sum(s for s, _ in pend)
    if left <= 0:
        return 0, False, popped, end
    first = pend[0][1]
    loop_before = any(branch_target(ins[k]) is not None and branch_target(ins[k]) <= ins[k].address
                      for k in range(start, first))
    branch_before = any(is_branch(ins[k]) for k in range(start, first))
    last_args = sum(s for s, k in pend if len(calls) < 2 or k > calls[-2])
    more = ins[stop - 1].mnemonic != "jsr" or left > last_args
    return left, loop_before or (branch_before and more), popped, end


if __name__ == "__main__":
    data = open(sys.argv[1], "rb").read()
    stats, bad = collections.Counter(), []
    for name, (a, b, _) in functions(data).items():
        if b <= a:
            continue
        ins = disasm(data[a:b], a)
        if not any(i.mnemonic == "unlk" for i in ins):
            continue
        left, wants, popped, _ = analyse(ins)
        if left:
            stats[(wants, popped)] += 1
            if wants != popped:
                bad.append(name)
    print("(rule says pop, game pops):", dict(stats))
    print("exceptions:", " ".join(bad))
