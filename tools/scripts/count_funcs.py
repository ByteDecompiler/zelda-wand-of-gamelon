"""Rough function count for an OS-9 68000 program module.
Usage: python -I count_funcs.py <module.mod>
Reads the header, takes the code part (up to the initialized-data offset), and counts 68000 opcodes
that start and end functions: link (stack frame set-up), rts (return), and the compiler-style
'movem.l regs,-(sp)' saves. Also lists any readable strings, which hint at the compiler and source.
"""
import re
import struct
import sys

data = open(sys.argv[1], "rb").read()
size, = struct.unpack_from(">I", data, 4)
exe, excpt, mem, stack, idata, irefs = struct.unpack_from(">6I", data, 0x30)
code_end = idata if 0x48 < idata < size else size
print(f"module size {size}, exec at 0x{exe:X}, data area {mem} bytes, init data at 0x{idata:X}, "
      f"code ~{(code_end - exe)} bytes")
code = data[exe:code_end]
words = [struct.unpack_from(">H", code, i)[0] for i in range(0, len(code) - 1, 2)]
link = sum(1 for w in words if w & 0xFFF8 == 0x4E50)
link_a5 = sum(1 for w in words if w == 0x4E55)
link_a6 = sum(1 for w in words if w == 0x4E56)
rts = sum(1 for w in words if w == 0x4E75)
movem = sum(1 for w in words if w == 0x48E7)  # movem.l <list>,-(sp)
unlk = sum(1 for w in words if w & 0xFFF8 == 0x4E58)
print(f"link: {link} (a5 {link_a5}, a6 {link_a6})  unlk: {unlk}  rts: {rts}  movem.l save: {movem}")
strs = re.findall(rb"[ -~]{6,}", data)
print(f"{len(strs)} strings; first ones and any compiler/source hints:")
for s in strs[:25]:
    print("  ", s.decode())
for s in strs:
    if re.search(rb"(?i)microware|ultra ?c|\.c\b|copyright|compiler|version|animation|magic", s):
        print("  hint:", s.decode())
