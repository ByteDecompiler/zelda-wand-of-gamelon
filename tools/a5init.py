"""Decode the initial values of cdi_zelda's globals and list them by name.

The program was linked by MPW, which packs the initial values of the A5 globals into a data stream that a
routine in code segment 8 ($1C9FC, called at startup through jump-table entry $60a) unpacks below A5.
This tool unpacks the same stream from your own module and writes:
  build/a5init.bin       the bytes below A5 as they are after startup (offset 0 = A5-$3828 in the English version)
  build/a5init.txt       each data symbol of config/symbols.txt: A5 offset, extent (up to the next
                         symbol), whether it has an initial value, and its first bytes
Pointers in the data are A5-relative: the unpacker adds A5 to them ("R" in the listing, value shown
before relocation, e.g. ffffe810 = A5-$17f0).

Usage: python3 tools/a5init.py [--version fr|de|nl] [module]   (another version writes to build/<version>/)
The output is game data: it stays in build/, which is never published.

Stream format (from the unpacker's code): blocks of
  size.l, base.l (0 = A5 - size), entries.w, unused.w, then `entries` entries:
  flags.b: bits 0-3 length (bit 4: one more byte follows; then if bit 11 of the length is set, another),
           bit 7: a repeat count follows (1-3 bytes, unused by the unpacker),
           bit 5: add the following `length`-byte value to the bytes in place instead of copying,
           bit 6: then add A5 to the long at the target (an A5-relative pointer);
  offset (2 bytes, bit 15: one more byte follows), then the bytes to copy (unless bit 5).
The block's memory is cleared first. The unpacker handles one block (none if its size is <= 0).
"""
import os
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
# the unpacker's first instructions: movem.l d0-d7/a0-a3,-(a7); clr.w d7; lea <stream>(pc),a0
UNPACKER = bytes.fromhex("48e7fff0424741fa")
sys.path.insert(0, os.path.join(ROOT, "tools"))
import build  # noqa: E402  (its --version option picks the version's symbols)


def stream(mod):
    """Where the packed data starts: the unpacker's lea points at it ($1CAF4 in the English version)."""
    at = mod.find(UNPACKER)
    return at + 8 + struct.unpack_from(">h", mod, at + 8)[0]


def unpack(mod):
    global BELOW
    p = stream(mod)
    BELOW = struct.unpack_from(">i", mod, p)[0]         # bytes of globals below A5: the first block's size
                                                        # ($3828 in the English version; the others differ)
    img = bytearray(BELOW)
    relocs = []

    def take(n):
        nonlocal p
        v = int.from_bytes(mod[p:p + n], "big")
        p += n
        return v

    for _ in range(1):                                  # the unpacker handles a single block
        size = struct.unpack_from(">i", mod, p)[0]
        if size <= 0:
            break
        p += 4
        base = take(4)
        base = BELOW - size if base == 0 else base       # offset of the block in img (A5-relative blocks)
        entries = take(2)
        take(2)
        img[base:base + size] = bytes(size)
        for _ in range(entries):
            flags = take(1)
            n = flags & 0xF
            if flags & 0x10:
                n = (n << 8) | take(1)
                if n & 0x800:
                    n = ((n & ~0x800) << 8) | take(1)
            if flags & 0x80:
                r = take(1)
                if r & 0x80:
                    r = ((r & 0x7F) << 8) | take(1)
                    if r & 0x4000:
                        take(1)
            off = take(2)
            if off & 0x8000:
                off = ((off & 0x7FFF) << 8) | take(1)
            at = base + off
            if flags & 0x20:
                v = take(n)
                old = int.from_bytes(img[at:at + n], "big")
                img[at:at + n] = ((old + v) % (1 << 8 * n)).to_bytes(n, "big")
            else:
                img[at:at + n] = mod[p:p + n]
                p += n
            if flags & 0x40:
                relocs.append(BELOW - at)
    return img, relocs


def main():
    args = [a for a in sys.argv[1:] if a != "--version" and a != build.VERSION]
    mod = open(args[0] if args else os.path.join(ROOT, "extract", *[v.upper() for v in build.SUB], "modules",
                                                 "cdi_zelda.mod"), "rb").read()
    img, relocs = unpack(mod)
    out_dir = os.path.join(ROOT, "build", *build.SUB)
    os.makedirs(out_dir, exist_ok=True)
    open(os.path.join(out_dir, "a5init.bin"), "wb").write(img)
    syms = {}
    for name, (kind, value) in build.load_symbols().items():
        if kind == "data":
            syms.setdefault(-value, []).append(name)
    offs = sorted(syms, reverse=True)
    out = []
    for i, o in enumerate(offs):
        nxt = offs[i + 1] if i + 1 < len(offs) else 0
        data = img[BELOW - o:BELOW - nxt]
        r = sum(1 for x in relocs if nxt < x <= o)
        out.append(f"-${o:04x} {o - nxt:5} {'I' if any(data) else ' '} {'R%-3d' % r if r else '    '} "
                   f"{'/'.join(syms[o]):28} {data[:32].hex() if any(data) else ''}")
    open(os.path.join(out_dir, "a5init.txt"), "w").write("\n".join(out) + "\n")
    where = os.path.relpath(out_dir, ROOT).replace(os.sep, "/")
    print(f"{len(relocs)} pointers; wrote {where}/a5init.bin and {where}/a5init.txt")


if __name__ == "__main__":
    main()
