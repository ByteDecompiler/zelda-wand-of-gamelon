"""Compiler fingerprints of a 68000 OS-9 module whose functions carry MacsBug names.
Usage: python -I fingerprint.py <module.mod> [function to print ...]
"""
import collections
import re
import struct
import sys

import capstone

data = open(sys.argv[1], "rb").read()
exe, = struct.unpack_from(">I", data, 0x30)
idata, = struct.unpack_from(">I", data, 0x40)

# MacsBug names: after rts (4E75) / jmp (a0) (4ED0) / rte (4E73): byte 0x80|len (or 0x80 + len byte), name,
# pad to even, then a word with the size of literal data that follows
names = []
for m in re.finditer(rb"(?:\x4e\x75|\x4e\xd0|\x4e\x73)([\x81-\x9f])([A-Za-z_%][A-Za-z0-9_%.]*)", data[:idata]):
    n = m.group(1)[0] & 0x7F
    name = m.group(2)[:n].decode()
    if len(name) != n:
        continue
    end = m.start(2) + n
    end += end & 1
    lit, = struct.unpack_from(">H", data, end)
    names.append((m.start() + 2, name, end + 2, lit))  # (code end, name, after word, literal bytes)
print(f"{len(names)} MacsBug names; literal-size words: "
      f"{collections.Counter(min(l, 1) for *_, l in names)} (0 = none, 1 = some)")

md = capstone.Cs(capstone.CS_ARCH_M68K, capstone.CS_MODE_BIG_ENDIAN | capstone.CS_MODE_M68K_000)
funcs, start = [], exe
for code_end, name, after, lit in names:
    funcs.append((name, start, code_end))
    start = after + lit
    start += start & 1

stats = collections.Counter()
first2 = collections.Counter()
last2 = collections.Counter()
ea_reg = collections.Counter()
show = set(sys.argv[2:])
for name, a, b in funcs:
    ins = list(md.disasm(data[a:b], a))
    if not ins:
        continue
    first2[" ; ".join(re.sub(r"#\$[0-9a-f]+|#-?\d+", "#N", f"{i.mnemonic} {i.op_str}") for i in ins[:2])] += 1
    last2[" ; ".join(re.sub(r"\$[0-9a-f]+|-?\d+", "N", f"{i.mnemonic} {i.op_str}") for i in ins[-3:])] += 1
    for k, i in enumerate(ins):
        mn = i.mnemonic
        stats[mn] += 1
        for r in re.findall(r"\((?:[^,()]*,)?(a[0-7]|sp|pc)[,)]", i.op_str):
            ea_reg[r] += 1
        if mn in ("jsr", "bsr.w", "bsr.b", "bsr") and k + 1 < len(ins):
            nxt = ins[k + 1]
            stats["after call: " + re.sub(r"#\$[0-9a-f]+", "#N", f"{nxt.mnemonic} {nxt.op_str}")] += 1
    if name in show:
        print(f"\n== {name} ({b - a} bytes)")
        for i in ins:
            print(f"  {i.address:6X}  {i.mnemonic:<8} {i.op_str}")

print("\nmost common first 2 instructions:")
for k, v in first2.most_common(6):
    print(f"  {v:4}  {k}")
print("most common last 3 instructions:")
for k, v in last2.most_common(6):
    print(f"  {v:4}  {k}")
print("memory accesses by base register:", dict(ea_reg.most_common()))
sz = {s: sum(v for k, v in stats.items() if k.startswith("move" + s)) for s in (".b", ".w", ".l")}
print("move sizes:", sz, " ext.w:", stats["ext.w"], " ext.l:", stats["ext.l"], " extb.l:", stats["extb.l"],
      " muls.w:", stats["muls.w"], " mulu.w:", stats["mulu.w"], " divs.w:", stats["divs.w"])
print("what follows a call:")
for k, v in sorted(((k, v) for k, v in stats.items() if k.startswith("after call")), key=lambda x: -x[1])[:8]:
    print(f"  {v:4}  {k[12:]}")
print("calls:", {k: stats[k] for k in ("jsr", "bsr.w", "bsr.b", "bsr") if stats[k]})
print("names (all):", " ".join(n for n, *_ in funcs))
