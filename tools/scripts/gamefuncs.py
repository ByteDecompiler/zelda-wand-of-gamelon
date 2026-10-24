"""Find the named functions of cdi_zelda.mod (MacsBug names) and disassemble them.
Usage: python -I gamefuncs.py <module.mod> --list            (all functions, smallest first)
       python -I gamefuncs.py <module.mod> <name> [<name> ...]  (disassembly)
Used as a library by compare.py: functions(data) -> {name: (start, code_end, end_with_name)}.
"""
import os
import re
import struct
import sys


def functions(data):
    """Map each MacsBug name to (code start, code end, end after the name and literal data)."""
    exe, = struct.unpack_from(">I", data, 0x30)
    idata, = struct.unpack_from(">I", data, 0x40)
    out, start = {}, exe
    for m in re.finditer(rb"(?:\x4e\x75|\x4e\xd0|\x4e\x73)([\x81-\x9f])([A-Za-z_%][A-Za-z0-9_%.]*)",
                         data[:idata]):
        n = m.group(1)[0] & 0x7F
        name = m.group(2)[:n].decode()
        if len(name) != n:
            continue
        end = m.start(2) + n
        end += end & 1
        lit, = struct.unpack_from(">H", data, end)
        after = end + 2 + lit
        after += after & 1
        out[name] = (start, m.start() + 2, after)
        start = after
    return out


def disasm(code, addr):
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_M68K, capstone.CS_MODE_BIG_ENDIAN | capstone.CS_MODE_M68K_000)
    return list(md.disasm(code, addr))


if __name__ == "__main__":
    data = open(sys.argv[1], "rb").read()
    funcs = functions(data)
    if sys.argv[2] == "--list":
        for name, (a, b, _) in sorted(funcs.items(), key=lambda kv: kv[1][1] - kv[1][0]):
            print(f"{b - a:6}  {a:6X}  {name}")
    root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    listed = os.path.join(root, "config", "functions.txt")
    if os.path.exists(listed):                         # also the fn_ and gap_ pieces of the split
        for line in open(listed):
            if line.strip() and not line.startswith("#"):
                a, b, kind, n = line.split()
                if kind != "func":
                    funcs[n] = (int(a, 16), int(b, 16), int(b, 16))
    for name in sys.argv[2:]:
        if name in funcs:
            a, b, _ = funcs[name]
            print(f"\n== {name} at {a:X} ({b - a} bytes)")
            for i in disasm(data[a:b], a):
                print(f"  {i.address:6X}  {i.bytes.hex():<20} {i.mnemonic:<8} {i.op_str}")
