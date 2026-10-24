"""Turn unar's AppleDouble '<name>.rsrc' files into raw resource forks '<name>.rf'.
Usage: python3 appledouble.py <folder>   (recursive)
"""
import os
import struct
import sys

for root, _, files in os.walk(sys.argv[1]):
    for f in files:
        if not f.endswith(".rsrc"):
            continue
        d = open(os.path.join(root, f), "rb").read()
        if d[:4] not in (b"\x00\x05\x16\x07", b"\x00\x05\x16\x00"):
            continue
        n, = struct.unpack_from(">H", d, 24)
        for k in range(n):
            eid, off, ln = struct.unpack_from(">III", d, 26 + 12 * k)
            if eid == 2 and ln:
                open(os.path.join(root, f[:-5] + ".rf"), "wb").write(d[off:off + ln])
