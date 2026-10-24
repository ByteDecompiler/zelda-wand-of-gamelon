"""Put a rebuilt cdi_zelda.mod into a copy of your disc image, to run it in a CD-i emulator.

Usage (from the project root):
  python3 tools/patch_disc.py [--version fr|de|nl] [--image disc/EN/<name>.bin] [--module build/cdi_zelda.mod]
                              [--out build/disc]
(--version de, for example: disc/DE/, build/de/cdi_zelda.mod and build/de/disc/ by default)

Copies the disc image (.bin + .cue) to build/disc/ (game data: never published), finds the sectors that
hold the original cdi_zelda module, writes the new module over them and recomputes each changed sector's
error detection and correction codes (EDC/ECC, Mode 2 Form 1), so that emulators and players accept it.
The new module may be bigger than the original: it uses the padding at the end of its file, then the free
sectors up to the next file (the file's size in its directory record is updated).
The original image is only read.
"""
import argparse
import glob
import os
import shutil
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SECTOR = 2352
DATA = 24                   # user data offset in a Mode 2 sector (sync 12, header 4, subheader 8)

# --- EDC (CRC-32, polynomial 0xD8018001, reflected) and ECC (Reed-Solomon P/Q over GF(2^8)) ---
EDC_TABLE = []
for i in range(256):
    e = i
    for _ in range(8):
        e = (e >> 1) ^ (0xD8018001 if e & 1 else 0)
    EDC_TABLE.append(e)

ECC_F, ECC_B = [0] * 256, [0] * 256
for i in range(256):
    j = (i << 1) ^ (0x11D if i & 0x80 else 0)
    ECC_F[i] = j
    ECC_B[i ^ j] = i


def edc(data):
    e = 0
    for b in data:
        e = EDC_TABLE[(e ^ b) & 0xFF] ^ (e >> 8)
    return e


def ecc_pq(sector, major_count, minor_count, major_mult, minor_inc, dest):
    """Compute P (86 x 24) or Q (52 x 43) parity over sector[12:], writing it at dest."""
    size = major_count * minor_count
    for major in range(major_count):
        index = (major >> 1) * major_mult + (major & 1)
        a = b = 0
        for _ in range(minor_count):
            t = sector[12 + index]
            index += minor_inc
            if index >= size:
                index -= size
            a ^= t
            b ^= t
            a = ECC_F[a]
        a = ECC_B[ECC_F[a] ^ b]
        sector[dest + major] = a
        sector[dest + major + major_count] = a ^ b


def fix_form1(sector):
    """Recompute EDC and ECC of a Mode 2 Form 1 sector in place."""
    struct.pack_into("<I", sector, 0x818, edc(sector[0x10:0x818]))
    header = sector[12:16]
    sector[12:16] = b"\0\0\0\0"          # Mode 2: the header is left out of the ECC
    ecc_pq(sector, 86, 24, 2, 86, 0x81C)
    ecc_pq(sector, 52, 43, 86, 88, 0x81C + 0xAC)
    sector[12:16] = header


def form1_sectors(img):
    """(sector number, user data) of every Form 1 sector, in disc order."""
    for n in range(len(img) // SECTOR):
        s = img[n * SECTOR:(n + 1) * SECTOR]
        if s[15] == 2 and not s[18] & 0x20:
            yield n, s


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--version", default="en", help="en, fr, de or nl: picks the default image, module and output")
    ap.add_argument("--image")
    ap.add_argument("--module")
    ap.add_argument("--out")
    args = ap.parse_args()
    sub = [] if args.version == "en" else [args.version]
    images = sorted(glob.glob(os.path.join(ROOT, "disc", args.version.upper(), "*.bin")))
    args.image = args.image or (images[0] if images else sys.exit(f"no image in disc/{args.version.upper()}/"))
    args.module = args.module or os.path.join(ROOT, "build", *sub, "cdi_zelda.mod")
    args.out = args.out or os.path.join(ROOT, "build", *sub, "disc")
    new = open(args.module, "rb").read()
    img = bytearray(open(args.image, "rb").read())

    # the Form 1 data stream, with each byte's place in the image
    places, stream = [], bytearray()
    for n, s in form1_sectors(img):
        places.append(n)
        stream += s[DATA:DATA + 2048]
    old_name = struct.unpack_from(">I", new, 0x0C)[0]
    name = new[old_name:old_name + 32].split(b"\0")[0]
    at = -1
    i = 0
    while True:
        i = stream.find(b"\x4a\xfc\x00\x01", i)
        if i < 0:
            break
        size, = struct.unpack_from(">I", stream, i + 4)
        noff, = struct.unpack_from(">I", stream, i + 0x0C)
        if stream[i + noff:i + noff + len(name) + 1] == name + b"\0":
            at = i
            break
        i += 2
    if at < 0:
        sys.exit(f"module {name.decode()} not found in the image")
    if at % 2048:
        sys.exit("the module doesn't start at a sector boundary")
    first = places[at // 2048]

    # the module's file: its directory record (LBN at +6, size at +14, both big-endian) and how far it can
    # grow (up to the next file on the disc)
    record, lbns = None, []
    for idx, n in enumerate(places[:at // 2048]):
        d = stream[idx * 2048:(idx + 1) * 2048]
        k = 0
        while k < 2048 and d[k] >= 34:
            lbn, = struct.unpack_from(">I", d, k + 6)
            lbns.append(lbn)
            if lbn == first and d[k + 33:k + 33 + d[k + 32]] == name:
                record = (idx, k)
            k += d[k]
    if record is None:
        sys.exit(f"no directory record for {name.decode()} at sector {first}")
    fsize, = struct.unpack_from(">I", stream, record[0] * 2048 + record[1] + 14)
    limit = min(l for l in lbns if l > first)          # the next file's first sector
    room = sum(1 for n in places if first <= n < limit) * 2048
    if len(new) > room:
        sys.exit(f"the new module ({len(new)} bytes) doesn't fit: {room} bytes up to the next file")
    if any(stream[at + fsize:at + room]) and len(new) > fsize:
        sys.exit("the sectors after the file aren't free")
    new_fsize = max(fsize, -(-len(new) // 2048) * 2048)

    # self-test of the EDC/ECC code on an original sector
    n0 = first
    test = bytearray(img[n0 * SECTOR:(n0 + 1) * SECTOR])
    fix_form1(test)
    if test != img[n0 * SECTOR:(n0 + 1) * SECTOR]:
        sys.exit("EDC/ECC self-test failed: the image's sectors don't match the computed codes")

    region = new + bytes(new_fsize - len(new))          # the module, then zeros to the end of the file
    stream[at:at + new_fsize] = region
    touched = set(range(at // 2048, (at + new_fsize) // 2048))
    if new_fsize != fsize:
        struct.pack_into(">I", stream, record[0] * 2048 + record[1] + 14, new_fsize)
        touched.add(record[0])
    changed = 0
    for idx in sorted(touched):
        n = places[idx]
        sec = bytearray(img[n * SECTOR:(n + 1) * SECTOR])
        if sec[DATA:DATA + 2048] != stream[idx * 2048:(idx + 1) * 2048]:
            sec[DATA:DATA + 2048] = stream[idx * 2048:(idx + 1) * 2048]
            fix_form1(sec)
            img[n * SECTOR:(n + 1) * SECTOR] = sec
            changed += 1

    os.makedirs(args.out, exist_ok=True)
    base = os.path.splitext(os.path.basename(args.image))[0]
    out_bin = os.path.join(args.out, base + " (mod).bin")
    open(out_bin, "wb").write(img)
    cue = open(os.path.splitext(args.image)[0] + ".cue").read()
    open(os.path.join(args.out, base + " (mod).cue"), "w").write(
        cue.replace(os.path.basename(args.image), os.path.basename(out_bin)))
    print(f"{name.decode()}: {len(new)} bytes ({len(new) - size:+d}) at sector {first}, file {new_fsize} bytes "
          f"(room for {room}); {changed} sectors changed")
    print(f"wrote {out_bin} and its .cue")


if __name__ == "__main__":
    main()
