"""Read files from a raw CD-i disc image (2352-byte Mode 2 sectors), for the content extractors.

Usage: python3 -I tools/cdi_fs.py [--version fr|de|nl]       lists the files on the disc

As a module:
  disc = Disc(path)            or Disc.open(version) for the image in disc/EN/ (or FR, DE, NL)
  disc.files                   {name: (first sector, size in bytes)} from the root directory
  disc.sectors(name)           yields (sector number, Sector) for each of the file's sectors
  Sector: .file .channel .submode .coding (the subheader), .form2, .audio, .video, .data, .eor, .eof,
          .payload (2048 bytes of a Form 1 sector, 2324 of a Form 2 one)

A real-time file (.rtr) interleaves several channels: each sector's subheader names its channel and
kind, and the program's play control lists (PCL) say which channel goes into which buffer. A record ends
with a sector whose submode has the end-of-record bit.

CD-i layout: the disc label is at sector 16 (its path table address at +148); the path table's first
entry is the root directory; directory records hold the first sector at +6, the size at +14 (both
big-endian) and the name (length at +32, name at +33). Sector numbers here are the image's.
"""
import glob
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SECTOR = 2352


class Sector:
    def __init__(self, raw):
        self.raw = raw
        self.file, self.channel, self.submode, self.coding = raw[16], raw[17], raw[18], raw[19]
        self.form2 = bool(self.submode & 0x20)
        self.eor = bool(self.submode & 0x01)            # end of record
        self.video = bool(self.submode & 0x02)
        self.audio = bool(self.submode & 0x04)
        self.data = bool(self.submode & 0x08)
        self.eof = bool(self.submode & 0x80)
        self.payload = raw[24:24 + (2324 if self.form2 else 2048)]


class Disc:
    def __init__(self, path):
        self.path = path
        self.f = open(path, "rb")
        label = self.read(16)[24:24 + 2048]
        assert label[1:6] == b"CD-I ", "no CD-i disc label at sector 16"
        path_table = struct.unpack_from(">I", label, 148)[0]
        root = struct.unpack_from(">I", self.read(path_table), 24 + 2)[0]
        self.files = {}
        d = self.read(root)[24:24 + 2048]
        k = 0
        while k < 2048 and d[k]:
            first, size = struct.unpack_from(">I", d, k + 6)[0], struct.unpack_from(">I", d, k + 14)[0]
            name = d[k + 33:k + 33 + d[k + 32]]
            if name not in (b"\0", b"\1"):
                self.files[name.decode("latin-1")] = (first, size)
            k += d[k]

    @classmethod
    def open(cls, version="en"):
        images = sorted(glob.glob(os.path.join(ROOT, "disc", version.upper(), "*.bin")))
        if not images:
            sys.exit(f"no disc image in disc/{version.upper()}/")
        return cls(images[0])

    def read(self, n):
        self.f.seek(n * SECTOR)
        return self.f.read(SECTOR)

    def sectors(self, name, start=0, count=None):
        """The file's sectors from its sector `start` on (`count` of them, or to the end of the file).
        A file's size counts 2048 bytes per sector, whatever the sector's form."""
        first, size = self.files[name]
        total = -(-size // 2048)
        end = total if count is None else min(total, start + count)
        self.f.seek((first + start) * SECTOR)
        for n in range(first + start, first + end):
            yield n, Sector(self.f.read(SECTOR))


VERSIONS = {"en": None, "fr": "FRENCH", "de": "GERMAN", "nl": "DUTCH"}     # the define include/version.h tests


def version_values(version="en"):
    """The numbers include/version.h defines for a version, e.g. version_values("de")["ROOM_SECTOR"]."""
    want = VERSIONS[version]
    values, active, chosen = {}, None, False
    for line in open(os.path.join(ROOT, "include", "version.h"), encoding="utf-8"):
        m = re.match(r"#(el)?if defined\((\w+)\)", line)
        if m and m.group(2) in VERSIONS.values():
            active = m.group(2) == want
            chosen |= active
        elif line.startswith("#else") and active is not None:
            active = not chosen
        elif line.startswith("#endif"):
            active = None
        elif active:
            m = re.match(r"#define\s+(\w+)\s+(\w+)", line)
            if m:
                values[m.group(1)] = int(m.group(2), 0)
    return values


def main():
    version = sys.argv[sys.argv.index("--version") + 1] if "--version" in sys.argv else "en"
    disc = Disc.open(version)
    print(os.path.basename(disc.path))
    for name, (first, size) in disc.files.items():
        print(f"  {name:<14} sector {first:6}  {size:10} bytes  ({-(-size // 2048)} sectors)")


if __name__ == "__main__":
    main()
