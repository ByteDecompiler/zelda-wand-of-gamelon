"""Find OS-9 modules (CD-i programs and libraries) in a raw 2352-byte CD-i image.
Usage: python -I cdi_modules.py <image.bin> <out_dir>
Joins the user data of the Form 1 (data) sectors, scans for module headers (sync 0x4AFC, valid header
parity) and writes each module to out_dir/<name>.mod.
"""
import os
import struct
import sys

TYPES = {1: "program", 2: "subroutine", 3: "multi", 4: "data", 11: "trap lib", 12: "system", 13: "file mgr",
         14: "driver", 15: "descriptor"}
LANGS = {0: "none", 1: "68000 code", 2: "basic I-code", 3: "pascal", 4: "C I-code", 5: "cobol", 6: "fortran"}


def form1_data(path):
    out = bytearray()
    with open(path, "rb") as f:
        while True:
            s = f.read(2352)
            if len(s) < 2352:
                break
            if s[15] != 2:              # mode byte: CD-i is mode 2
                continue
            submode = s[18]
            if submode & 0x20:          # form 2 (audio / video / real-time), not files
                continue
            out += s[24:24 + 2048]
    return bytes(out)


def modules(data):
    i = 0
    while True:
        i = data.find(b"\x4a\xfc", i)
        if i < 0 or i + 0x30 > len(data):
            return
        hdr = data[i:i + 0x30]
        par = 0
        for k in range(0, 0x30, 2):
            par ^= struct.unpack_from(">H", hdr, k)[0]
        size, = struct.unpack_from(">I", hdr, 4)
        if par == 0xFFFF and 0x30 < size < 0x2000000 and i + size <= len(data):
            name_off, = struct.unpack_from(">I", hdr, 0x0C)
            name = data[i + name_off:i + name_off + 32].split(b"\0")[0].decode("latin-1", "replace") \
                if name_off < size else "?"
            yield i, size, name, hdr[0x12], hdr[0x13]
            i += size
        else:
            i += 2


def main():
    img, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    data = form1_data(img)
    print(f"form 1 data: {len(data) / 1e6:.1f} MB")
    for off, size, name, tl, attr in modules(data):
        t, l = tl >> 4, tl & 0xF
        print(f"{off:9X} {size:9d}  {name:<20} {TYPES.get(t, t)}, {LANGS.get(l, l)}")
        safe = "".join(c if c.isalnum() or c in "._-" else "_" for c in name) or f"mod_{off:X}"
        open(os.path.join(out, f"{safe}.mod"), "wb").write(data[off:off + size])


if __name__ == "__main__":
    main()
