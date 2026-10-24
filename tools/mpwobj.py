"""Read MPW object files (.o) as written by MPW C 3.x.

read(path) -> {module name: Module}, where a Module has
  code   the module's bytes (for a C function: the code, then its MacsBug name, pad and literal-size word)
  refs   [(offset, symbol name, a5)] for every 16-bit field the linker fills in: a5=True for an A5-relative
         reference (a global, field = offset within it), False for a code reference (a call, field = 0)
  data   True for a data module (a C global or string constants)
  size   the declared size of a data module
Record layouts follow what these compilers write, including the extra records of -sym on.
"""
import struct
import sys


class Module:
    def __init__(self, name, data, size):
        self.name, self.data, self.size = name, data, size
        self.code = bytearray()
        self.refs = []


def read(path):
    d = open(path, "rb").read()
    names, mods, cur, pos = {}, {}, None, 0
    while pos < len(d):
        kind, flags = d[pos], d[pos + 1] if pos + 1 < len(d) else 0
        if kind == 0:                                   # Pad
            pos += 1
            continue
        if kind == 1:                                   # First
            size = 4
        elif kind == 2:                                 # Last
            break
        elif kind == 5:                                 # Module: id, then segment id (code) or size (data)
            mid, = struct.unpack_from(">H", d, pos + 2)
            msize = struct.unpack_from(">H", d, pos + 4)[0] if flags & 1 else 0
            cur = Module(mid, bool(flags & 1), msize)
            mods[mid] = cur
            size = 6
        elif kind in (6, 11, 14):                       # EntryPoint, Filename, ModuleEnd (-sym)
            size = 8
        elif kind == 7:                                 # Size
            size = 6
        elif kind == 16:                                # block end (-sym): id, two longs
            size = 12
        else:                                           # records that carry their own size
            size, = struct.unpack_from(">H", d, pos + 2)
            if size < 4:
                sys.exit(f"{path}: can't parse record type {kind} at {pos:#x}")
            body = d[pos + 4:pos + size]
            if kind == 4:                               # Dictionary: first id, then Pascal strings
                nid, = struct.unpack_from(">H", body, 0)
                k = 2
                while k < len(body):
                    n = body[k]
                    names[nid] = body[k + 1:k + 1 + n].decode("mac_roman")
                    nid, k = nid + 1, k + 1 + n
            elif kind == 8 and cur is not None:         # Contents: [offset], [repeat count], bytes
                off, k, rep = 0, 0, 1
                if flags & 0x08:
                    off, = struct.unpack_from(">I", body, 0)
                    k = 4
                if flags & 0x10:
                    rep, = struct.unpack_from(">H", body, k)
                    k += 2
                data = body[k:] * rep
                cur.code.extend(b"\0" * max(0, off + len(data) - len(cur.code)))
                cur.code[off:off + len(data)] = data
            elif kind == 9 and cur is not None:         # Reference: symbol id, offsets
                sid, = struct.unpack_from(">H", body, 0)
                # Seen: flags $90 (A5-relative) and $10 (code), 16-bit offsets and fields. Bit 3 is
                # assumed to mean 32-bit offsets in the list (not seen yet).
                step = 4 if flags & 0x08 else 2
                for k in range(2, len(body) - step + 1, step):
                    off = int.from_bytes(body[k:k + step], "big")
                    cur.refs.append((off, sid, bool(flags & 0x80)))
            elif kind == 10:                            # ComputedRef: not supported yet
                sys.exit(f"{path}: computed reference (record type 10) not supported yet")
        pos += size
    out = {}
    for mid, mod in mods.items():                       # names may come after the records that use them
        mod.name = names.get(mid, f"#{mid}")
        mod.refs = [(off, names.get(sid, f"#{sid}"), a5) for off, sid, a5 in mod.refs]
        out[mod.name] = mod
    return out


def code_only(mod):
    """A code module's bytes without the trailing MacsBug name (len byte, name, pad, literal-size word)."""
    n = len(mod.name)
    start = len(mod.code) - (1 + n + ((1 + n) & 1) + 2)
    name = mod.name.encode("mac_roman")
    if start < 0 or mod.code[start] & 0x7F != n or mod.code[start + 1:start + 1 + n] != name:
        return bytes(mod.code)                      # compiled with -mbg off: no name
    return bytes(mod.code[:start])
