"""Install MPW 3.3 plus the MPW C compilers into a folder of an HFS disk image (for Basilisk II).

Run inside WSL (needs hfsutils):
    python3 install_mpw.py <image.dsk> <HFS folder, e.g. MacOS7:MPW> <mpw33/MPW folder> <C 3.2 floppy .hfs> <C 3.1 floppy .hfs>

The MPW folder is unar's output: each file is '<name>' (data fork, may be missing) plus '<name>.rsrc'
(AppleDouble with Finder info and resource fork). '<name>.rf' files are skipped.
Files go in as MacBinary II through 'hcopy -m', so type/creator and both forks survive.
"""
import os
import struct
import subprocess
import sys
import tempfile
import time

MAC_DATE = int(time.time()) + 2082844800  # now, in Mac time (seconds since 1 Jan 1904)

def crc_ccitt(data):
    crc = 0
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) if crc & 0x8000 else crc << 1
            crc &= 0xFFFF
    return crc


def macbinary(name, finfo, data, rsrc):
    hdr = bytearray(128)
    hdr[1] = len(name)
    hdr[2:2 + len(name)] = name
    hdr[65:73] = finfo[0:8]          # type, creator
    hdr[73] = finfo[8]               # Finder flags, high byte
    hdr[83:91] = struct.pack(">II", len(data), len(rsrc))
    # Creation and modification dates (seconds since 1904). MPW rejects tools with a zero date as damaged.
    hdr[91:99] = struct.pack(">II", MAC_DATE, MAC_DATE)
    hdr[101] = finfo[9]              # Finder flags, low byte
    hdr[122] = hdr[123] = 129        # MacBinary II
    hdr[124:126] = struct.pack(">H", crc_ccitt(bytes(hdr[:124])))
    pad = lambda b: b + b"\0" * (-len(b) % 128)
    return bytes(hdr) + pad(data) + pad(rsrc)


def appledouble(path):
    """Return (finder info, resource fork) from an AppleDouble file."""
    finfo, rsrc = b"\0" * 32, b""
    if not os.path.exists(path):
        return finfo, rsrc
    d = open(path, "rb").read()
    n, = struct.unpack_from(">H", d, 24)
    for k in range(n):
        eid, off, ln = struct.unpack_from(">III", d, 26 + 12 * k)
        if eid == 9:
            finfo = d[off:off + ln].ljust(32, b"\0")
        elif eid == 2:
            rsrc = d[off:off + ln]
    return finfo, rsrc


def h(*args):
    subprocess.run(["h" + args[0], *args[1:]], check=True, stdout=subprocess.DEVNULL)


def mac(path):
    return path.encode("mac_roman")


def copy_tree(host_root, hfs_root, tmp):
    for root, dirs, files in os.walk(host_root):
        rel = os.path.relpath(root, host_root)
        dest = hfs_root if rel == "." else hfs_root + ":" + rel.replace("/", ":")
        if rel != ".":
            h("mkdir", mac(dest))
        names = {f[:-5] if f.endswith(".rsrc") else f for f in files if not f.endswith(".rf")}
        for name in sorted(names):
            data_path = os.path.join(root, name)
            data = open(data_path, "rb").read() if os.path.isfile(data_path) else b""
            finfo, rsrc = appledouble(data_path + ".rsrc")
            open(tmp, "wb").write(macbinary(mac(name), finfo, data, rsrc))
            h("copy", "-m", tmp, mac(dest + ":"))


def main():
    image, folder, mpw_dir, c32_floppy, c31_floppy = sys.argv[1:6]
    tmp = tempfile.mktemp(suffix=".bin")
    h("mount", image)
    h("mkdir", mac(folder))
    copy_tree(mpw_dir, folder, tmp)
    # The compilers: MPW looks tools up by file name, so C 3.2 becomes "C" and C 3.1 "C31".
    for floppy, src, dst in ((c32_floppy, ":C 3.2", "C"), (c31_floppy, ":Tools:C", "C31")):
        h("mount", floppy)
        h("copy", "-m", src, tmp)
        h("mount", image)
        h("copy", "-m", tmp, mac(folder + ":Tools:" + dst))
    h("umount")
    os.remove(tmp)


if __name__ == "__main__":
    main()
