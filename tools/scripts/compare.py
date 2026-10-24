"""Compare functions compiled by MPW C (an MPW .o file) with the same-named functions in the game.
Usage: python -I compare.py <game module.mod> <file.o> [-v] [-d] [--fix] [--pop f,g] [--nopop h]
For each code module in the .o that has a same-named game function, prints MATCH or the two disassemblies
side by side. Fields the linker fills in (offsets of globals and call targets, found from the .o's Reference
records) are not compared, and a JSR's opcode may differ (the linker picks JSR d16(PC) or JSR d16(A5)).
-v prints matching functions too, -d prints only the differing lines.
--fix applies the two corrections that turn C 3.2 output into the game compiler's (see fixup()), with
--pop / --nopop overriding the end-of-function pop rule for the named functions.
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gamefuncs import disasm, functions  # noqa: E402
from stackdepth import analyse  # noqa: E402

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import mpwobj  # noqa: E402


def read_obj(path):
    """Return {code module name: (code bytes without the MacsBug name, offsets of bytes the linker fills in)}."""
    out = {}
    for name, mod in mpwobj.read(path).items():
        if not mod.data:
            out[name] = (mpwobj.code_only(mod), {o + k for o, _, _ in mod.refs for k in (0, 1)})
    return out


def fixup(code, fill, force=None):
    """Turn C 3.2 output into what the game's compiler emits:
    1. 'addq.w #n,a7' becomes 'addq.l #n,a7' (same size).
    2. Arguments still on the stack at the epilogue get popped there ('addq.l' up to 8 bytes, else 'lea')
       when the game compiler's rule (stackdepth.analyse) says so, or when force is True.
    Returns (code, fill, note) or raises ValueError when a branch can no longer reach its target."""
    ins = disasm(code, 0)
    code = bytearray(code)
    notes = []
    for i in ins:
        w = (i.bytes[0] << 8) | i.bytes[1]
        if (w & 0xF1FF) == 0x504F:                    # addq.w #n,a7
            w = (w & ~0x00C0) | 0x0080
            code[i.address:i.address + 2] = w.to_bytes(2, "big")
            notes.append("addq.l")
    if not any(i.mnemonic == "unlk" for i in ins):
        return bytes(code), fill, notes
    left, wants, popped, end = analyse(ins)
    if force is not None:
        wants = force
    if left and wants and not popped:
        at = ins[end].address
        pop = ((0x508F | ((left & 7) << 9)).to_bytes(2, "big") if left <= 8
               else b"\x4f\xef" + left.to_bytes(2, "big"))
        for i in ins[:end]:                           # forward branches that jump to or past the insertion
            w = (i.bytes[0] << 8) | i.bytes[1]
            if (w & 0xF000) == 0x6000 and (w & 0xFF) not in (0x00, 0xFF):
                disp = (w & 0xFF) - 256 if w & 0x80 else w & 0xFF
                if i.address + 2 + disp >= at:
                    disp += len(pop)
                    if disp > 127:
                        raise ValueError("short branch out of range after inserting the pop")
                    code[i.address + 1] = disp & 0xFF
            elif (w & 0xF000) == 0x6000 or (w & 0xF0F8) == 0x50C8:   # Bcc.w / DBcc: 16-bit displacement
                disp = int.from_bytes(code[i.address + 2:i.address + 4], "big", signed=True)
                if i.address + 2 + disp >= at:
                    code[i.address + 2:i.address + 4] = (disp + len(pop)).to_bytes(2, "big", signed=True)
        code[at:at] = pop
        fill = {o + len(pop) if o >= at else o for o in fill}
        notes.append(f"pop {left}")
    return bytes(code), fill, notes


def linker_blind(line):
    """An instruction's text without the parts the linker fills in (for marking differing lines)."""
    text = line.split(" ", 1)[-1].strip()
    text = re.sub(r"-?\$[0-9a-f]+\(a5\)", "G(a5)", text)
    return re.sub(r"^jsr .*", "jsr X", text)


def same(game, mine, fill):
    if len(game) != len(mine):
        return False
    for i, (g, m) in enumerate(zip(game, mine)):
        if i in fill or g == m:
            continue
        # JSR d16(PC) = 4EBA vs JSR d16(A5) = 4EAD: second opcode byte of a call the linker resolves
        if i + 1 in fill and i >= 1 and game[i - 1] == mine[i - 1] == 0x4E and {g, m} <= {0xBA, 0xAD}:
            continue
        return False
    return True


def main():
    game = open(sys.argv[1], "rb").read()
    funcs = functions(game)
    args = sys.argv[3:]
    verbose = "-v" in args
    only_diffs = "-d" in args
    fix = "--fix" in args
    force = {}
    for flag, value in (("--pop", True), ("--nopop", False)):
        if flag in args:
            force.update({n: value for n in args[args.index(flag) + 1].split(",")})
    matched = total = 0
    for name, (mine, fill) in read_obj(sys.argv[2]).items():
        if name not in funcs:
            print(f"-- {name}: no such game function")
            continue
        a, b, _ = funcs[name]
        theirs = game[a:b]
        total += 1
        note = ""
        if fix:
            try:
                mine, fill, notes = fixup(mine, fill, force.get(name))
                note = f" [fixed: {', '.join(notes)}]" if notes else ""
            except ValueError as e:
                note = f" [fix-up failed: {e}]"
        ok = same(theirs, mine, fill)
        matched += ok
        print(f"{'MATCH' if ok else 'DIFF '} {name} (game {len(theirs)} bytes, mine {len(mine)}){note}")
        if ok and not verbose:
            continue
        g = [f"{i.bytes.hex():<12} {i.mnemonic} {i.op_str}" for i in disasm(theirs, 0)]
        m = [f"{i.bytes.hex():<12} {i.mnemonic} {i.op_str}" for i in disasm(mine, 0)]
        for k in range(max(len(g), len(m))):
            gl, ml = (g[k] if k < len(g) else ""), (m[k] if k < len(m) else "")
            mark = " " if linker_blind(gl) == linker_blind(ml) else "*"
            if mark == "*" or not only_diffs:
                print(f"   {mark} {gl:<44} | {ml}")
    print(f"{matched}/{total} functions match")


main()
