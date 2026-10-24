import struct, sys
d = open(sys.argv[1], "rb").read()
dl, rl = struct.unpack_from(">II", d, 83)
r0 = 128 + ((dl + 127) // 128) * 128
open(sys.argv[2], "wb").write(d[r0:r0 + rl])
print(sys.argv[2], "resource fork", rl, "bytes")
