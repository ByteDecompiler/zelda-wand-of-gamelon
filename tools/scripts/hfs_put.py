"""Put a host text file into an HFS disk image as a Mac text file (MacRoman, CR line endings).

Run inside WSL (needs hfsutils):
    python3 hfs_put.py <image.dsk> <HFS:path:Name> <TYPE> <CREATOR> <host file (UTF-8)>
"""
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(__file__))
from install_mpw import macbinary  # noqa: E402

image, hfs_path, ftype, creator, host = sys.argv[1:6]
text = open(host, encoding="utf-8").read().replace("\r\n", "\n").replace("\n", "\r").encode("mac_roman")
finfo = (ftype.ljust(4) + creator.ljust(4)).encode("mac_roman") + b"\0" * 24
tmp = tempfile.mktemp(suffix=".bin")
open(tmp, "wb").write(macbinary(b"x", finfo, text, b""))
subprocess.run(["hmount", image], check=True, stdout=subprocess.DEVNULL)
subprocess.run(["hdel", hfs_path.encode("mac_roman")], stderr=subprocess.DEVNULL)
subprocess.run(["hcopy", "-m", tmp, hfs_path.encode("mac_roman")], check=True)
subprocess.run(["humount"], check=True)
os.remove(tmp)
