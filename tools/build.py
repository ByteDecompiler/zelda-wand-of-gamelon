"""Build cdi_zelda.mod from the decompiled C (src/) and the not-yet-decompiled pieces (asm/, bin/).

Usage (in WSL, from the project root):
  python3 tools/build.py [--no-compile] [--diff] [--mod] [--version fr|de|nl]
--diff shows each failing function next to the game's code (needs Capstone: run it with
~/foe_tools/venv/bin/python -I). --mod builds a mod: functions whose C was changed are kept (they must keep
their size), and the result is no longer the original (then use tools/patch_disc.py to try it).
--version fr, de or nl builds the French, German or Dutch version: the C is compiled with -d FRENCH, GERMAN
or DUTCH (include/version.h); the version's own files come from config/<version>/ (its symbols.txt lists
only the globals that moved; files it lacks are shared), asm/<version>/ and bin/<version>/; objects go to
build/<version>/obj/ and the result to build/<version>/cdi_zelda.mod.

1. Compiles every src/*.c with MPW C 3.0 (tool C30) in the emulated Mac (emu/mpwrun.sh), with the
   headers from include/. Objects land in build/obj/. --no-compile reuses them.
2. Links: each code module in the objects replaces the same-named piece of config/functions.txt.
   References are resolved by name: globals and jump-table entries from config/symbols.txt, game
   functions from config/functions.txt. A call to a jump-table entry becomes JSR d16(A5), a call to a
   game function JSR d16(PC). Every other piece comes from asm/<name>.s.
3. Joins bin/header.bin, the pieces and bin/tail.bin, appends the OS-9 module CRC, writes
   build/cdi_zelda.mod and checks its SHA-1 against config/cdi_zelda.sha1.
"""
import glob
import hashlib
import os
import re
import shutil
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import mpwobj  # noqa: E402

COMPILER = "C30"
# PC-relative opcode word -> the same instruction A5-relative (for references to jump-table entries)
PC_TO_A5 = {0x4EBA: 0x4EAD, 0x487A: 0x486D, **{0x41FA | r << 9: 0x41ED | r << 9 for r in range(8)}}


VERSION = sys.argv[sys.argv.index("--version") + 1] if "--version" in sys.argv else "en"
SUB = [] if VERSION == "en" else [VERSION]
LANGUAGES = {"fr": "FRENCH", "de": "GERMAN", "nl": "DUTCH"}       # the define include/version.h tests              # subdirectory of the version's own files
OUT = os.path.join(ROOT, "build", *SUB, "cdi_zelda.mod")


def path(*p):
    return os.path.join(ROOT, *p)


def config(name):
    """The version's own config/<version>/<name>, or the shared config/<name>."""
    own = path("config", *SUB, name)
    return own if os.path.exists(own) else path("config", name)


def load_pieces():
    pieces = []
    for line in open(config("functions.txt")):
        if line.strip() and not line.startswith("#"):
            start, end, kind, name = line.split()
            pieces.append((int(start, 16), int(end, 16), kind, name))
    return pieces


def load_symbols():
    """config/symbols.txt: '<kind> <name> <value>' with kind data (A5 offset), jt (jump-table A5 offset of a
    routine with no address of its own here: the library in the module's tail) or code (module address of a
    routine inside an assembly piece). Another version's config/<version>/symbols.txt changes some values."""
    syms = {}
    files = [path("config", "symbols.txt")] + ([path("config", *SUB, "symbols.txt")] if SUB else [])
    for file in files:
        own = file != files[0]
        for n, line in enumerate(open(file), 1):
            line = line.split("#", 1)[0].strip()
            if not line:
                continue
            fields = line.split()
            where = os.path.relpath(file, ROOT)
            if len(fields) != 3 or fields[0] not in ("data", "jt", "code"):
                sys.exit(f"{where}:{n}: bad symbol line")
            kind, name, value = fields
            if (name in syms) != own or own and syms[name][0] != kind:
                sys.exit(f"{where}:{n}: {'unknown' if own else 'duplicate'} symbol '{name}'")
            syms[name] = (kind, int(value, 0))
    return syms


def load_jump_table():
    """config/jumptable.txt (written by split.py): module address -> A5 offset of its jump-table entry."""
    jt = {}
    for line in open(config("jumptable.txt")):
        if line.strip() and not line.startswith("#"):
            off, seg, addr = line.split()
            jt[int(addr, 16)] = int(off, 0)
    return jt


def load_segments():
    """config/segments.txt: the first address of each code segment, in order."""
    return sorted(int(line.split()[1], 16) for line in open(config("segments.txt"))
                  if line.strip() and not line.startswith("#"))


def segment(segs, addr):
    return sum(1 for s in segs if s <= addr)


def asm_bytes(name):
    out = bytearray()
    for line in open(path("asm", *SUB, f"{name}.s")):
        m = re.match(r"\s*dc\.w\s+([^;]+)", line)
        if m:
            for w in m.group(1).split(","):
                out += int(w.strip().lstrip("$"), 16).to_bytes(2, "big")
    return bytes(out)


def compile_sources():
    sources = sorted(glob.glob(path("src", "*.c")))
    job = path("build", *SUB, "job")
    shutil.rmtree(job, ignore_errors=True)
    os.makedirs(job)
    for f in glob.glob(path("include", "*.h")):
        shutil.copy(f, job)
    # Mac file names have at most 31 characters, so each source is compiled as s<number>.c
    for n, s in enumerate(sources):
        shutil.copy(s, os.path.join(job, f"s{n}.c"))
    with open(os.path.join(job, "build.mpw"), "w", newline="\n") as f:
        for n, s in enumerate(sources):
            # a file can ask for extra options, e.g. /* MPW C options: -mbg off */ (no MacsBug names)
            m = re.search(r"MPW C options:\s*(.*?)\s*\*/", open(s, encoding="utf-8").read())
            opts = f" {m.group(1)}" if m else ""
            if VERSION != "en":
                opts += f" -d {LANGUAGES[VERSION]}"
            f.write(f"{COMPILER}{opts} -o s{n}.o s{n}.c\n")
    r = subprocess.run([path("emu", "mpwrun.sh"), job], capture_output=True, text=True)
    status = open(os.path.join(job, "status.txt")).read().strip() if os.path.exists(
        os.path.join(job, "status.txt")) else "none"
    missing = [s for n, s in enumerate(sources) if not os.path.exists(os.path.join(job, f"s{n}.o"))]
    if status != "0" or missing:
        log = os.path.join(job, "log.txt")
        errors = open(log, errors="replace").read() if os.path.exists(log) else ""
        names = "".join(f"\n  s{sources.index(s)}.c = {os.path.relpath(s, ROOT)}" for s in missing)
        sys.exit(f"compile failed (MPW status {status}){names}\n{errors}{r.stdout}{r.stderr}")
    obj = path("build", *SUB, "obj")
    shutil.rmtree(obj, ignore_errors=True)
    os.makedirs(obj)
    for n, s in enumerate(sources):
        base = os.path.splitext(os.path.basename(s))[0]
        shutil.move(os.path.join(job, f"s{n}.o"), os.path.join(obj, f"{base}.o"))
    print(f"compiled {len(sources)} C files with {COMPILER}")


def link_module(mod, start, slot, funcs, syms, jt, segs):
    """Resolve a compiled function's references for its place at 'start'. Returns its bytes.
    A call to a routine in the same segment is PC-relative; to one in another segment, it goes through
    the routine's jump-table entry (MPW's linker does the same)."""
    code = bytearray(mod.code)
    errors = []
    for off, sym, a5 in mod.refs:
        field = int.from_bytes(code[off:off + 2], "big", signed=True)
        if a5:
            # a global, a library routine, or the address of a game function (e.g. a signal handler passed
            # to a routine): MPW takes a function's address from its jump-table entry
            if sym in syms and syms[sym][0] != "code":
                value = syms[sym][1] + field
            elif sym in funcs and jt.get(funcs[sym]) is not None:
                value = jt[funcs[sym]] + field
            else:
                errors.append(f"unknown global or jump-table entry '{sym}'")
                continue
        elif sym in funcs and segment(segs, funcs[sym]) == segment(segs, start):
            value = funcs[sym] - (start + off) + field        # PC-relative: PC = address of the field
        elif sym in funcs or (sym in syms and syms[sym][0] == "jt"):
            entry = jt.get(funcs[sym]) if sym in funcs else syms[sym][1]
            if entry is None:
                errors.append(f"'{sym}' is in another segment and has no jump-table entry")
                continue
            op = int.from_bytes(code[off - 2:off], "big")
            if op not in PC_TO_A5:
                errors.append(f"can't turn the instruction ${op:04X} into an A5 reference ('{sym}')")
                continue
            code[off - 2:off] = PC_TO_A5[op].to_bytes(2, "big")
            value = entry + field
        else:
            errors.append(f"unknown function '{sym}'")
            continue
        if not -0x8000 <= value <= 0x7FFF:
            errors.append(f"'{sym}' is out of 16-bit range")
            continue
        code[off:off + 2] = value.to_bytes(2, "big", signed=True)
    if len(code) != slot:
        errors.append(f"is {len(code)} bytes, its place in the module is {slot}")
    return bytes(code), errors


def disasm_lines(code, start):
    """Capstone stops at a switch table, so after each jmp (pc,dN) read the table's words (up to its lowest
    target) and go on after it."""
    sys.path.insert(0, path("tools", "scripts"))
    from gamefuncs import disasm
    out, a = [], 0
    while a < len(code):
        ins = disasm(code[a:a + 10], start + a)
        if not ins:
            out.append(f"{code[a:a + 2].hex():<12} dc.w"); a += 2; continue
        i = ins[0]
        out.append(f"{i.bytes.hex():<12} {i.mnemonic} {i.op_str}")
        a += i.size
        if i.mnemonic == "rts":
            break                                       # the MacsBug name follows
        if i.mnemonic.startswith("jmp") and "pc" in i.op_str:
            base, low = a - 2, None                     # entries count from the jmp's extension word
            while a + 2 <= len(code) and (low is None or a < low):
                t = base + int.from_bytes(code[a:a + 2], "big", signed=True)
                low = t if low is None else min(low, t)
                out.append(f"{code[a:a + 2].hex():<12} case -> ${start + t:X}")
                a += 2
    return out


def show_diff(name, mine, theirs, start):
    m = disasm_lines(mine, start)
    g = disasm_lines(theirs, start)
    print(f"    {'game':<46} | mine")
    for k in range(max(len(g), len(m))):
        gl, ml = (g[k] if k < len(g) else ""), (m[k] if k < len(m) else "")
        print(f"  {' ' if gl == ml else '*'} {gl:<46} | {ml}")
    # Capstone stops at data it can't decode (switch tables, strings), so list the raw bytes that differ too
    diffs = [k for k in range(min(len(mine), len(theirs))) if mine[k] != theirs[k]]
    for k in diffs[:16]:
        print(f"    byte at ${start + k:X} (+${k:X}): game {theirs[k]:02x}, mine {mine[k]:02x}")
    if len(diffs) > 16:
        print(f"    ... {len(diffs) - 16} more differing bytes")


def os9_crc(data):
    crc = 0xFFFFFF
    for b in data:
        crc ^= b << 16
        for _ in range(8):
            crc <<= 1
            if crc & 0x1000000:
                crc ^= 0x800063
        crc &= 0xFFFFFF
    return (~crc & 0xFFFFFF).to_bytes(3, "big")


def load_relocs():
    """config/relocs.txt (written by tools/relocs.py): PC-relative references from the copied code (assembly
    pieces, the tail) into another piece: (instruction address, kind 'w' or 'b', target address)."""
    out = []
    for line in open(config("relocs.txt")):
        line = line.split("#", 1)[0].split()
        if line:
            out.append((int(line[0], 16), line[1], int(line[2], 16)))
    return out


def header_parity(mod):
    """OS-9 module header check word (+$2e): the complement of the XOR of the header's first 23 words."""
    p = 0
    for k in range(0, 0x2E, 2):
        p ^= struct.unpack_from(">H", mod, k)[0]
    return ~p & 0xFFFF


def build_mod(pieces, syms, funcs_old, jt_old, segs_old, compiled):
    """A mod build: every C function comes from its compiled code, whatever its size; the code after a
    function that grew or shrank moves, and everything that refers to moved code is adjusted:
    calls between C functions (relinked), references from the assembly pieces and the tail
    (config/relocs.txt), the jump table in the initialized data, and the module header."""
    header = bytearray(open(path("bin", *SUB, "header.bin"), "rb").read())
    tail = bytearray(open(path("bin", *SUB, "tail.bin"), "rb").read())
    old_tail = pieces[-1][1]
    missing = [name for _, _, kind, name in pieces if kind in ("func", "fn") and name not in compiled]
    if missing:
        sys.exit(f"a mod build needs every C function compiled; missing: {', '.join(missing)}")

    # 1. the new layout
    layout = []                                         # (old start, old end, new start, new size)
    at = len(header)
    for start, end, kind, name in pieces:
        size = len(compiled[name].code) if name in compiled else end - start
        layout.append((start, end, at, size))
        at += size
    new_tail = at

    def new_addr(old):
        if old < len(header):
            return old
        if old >= old_tail:
            return new_tail + old - old_tail
        for start, end, new, size in layout:
            if start <= old < end:
                if old != start and end - start != size:
                    sys.exit(f"${old:X} points into a function that changed size")
                return new + old - start
        sys.exit(f"${old:X} is not in any piece")

    funcs = {name: new_addr(a) for name, a in funcs_old.items()}
    jt = {new_addr(a): off for a, off in jt_old.items()}
    segs = [new_addr(s) for s in segs_old]

    # 2. the pieces
    out = bytearray(header)
    errors = 0
    changed = []
    for (start, end, kind, name), (_, _, new, size) in zip(pieces, layout):
        if name in compiled:
            code, errs = link_module(compiled[name], new, size, funcs, syms, jt, segs)
            for e in errs:
                print(f"error: {name}: {e}")
            errors += len(errs)
            # changed by the mod (not just moved): it doesn't match the game when linked at its old place
            if size != end - start or link_module(compiled[name], start, size, funcs_old, syms, jt_old,
                                                  segs_old)[0] != asm_bytes(name):
                changed.append(name)
        else:
            code = asm_bytes(name)
        out += code
    out += tail
    if errors:
        sys.exit(f"{errors} link error(s)")

    # 3. references from the copied code (assembly pieces, tail) into other pieces
    for addr, kind, target in load_relocs():
        a, t = new_addr(addr), new_addr(target)
        disp = t - (a + 2)
        if kind == "w":
            if not -0x8000 <= disp <= 0x7FFF:
                sys.exit(f"the reference at ${addr:X} can't reach ${target:X} any more")
            out[a + 2:a + 4] = disp.to_bytes(2, "big", signed=True)
        else:
            if not -0x80 <= disp <= 0x7F or disp in (0, -1):
                sys.exit(f"the short branch at ${addr:X} can't reach ${target:X} any more")
            out[a + 1] = disp & 0xFF

    # 4. the jump table (in the initialized data) and the module header
    delta = new_tail - old_tail
    old_idata = struct.unpack_from(">I", header, 0x40)[0]
    idata = old_idata + delta
    size = struct.unpack_from(">I", out, idata + 4)[0]
    for off in range(8, size, 8):
        entry = idata + 8 + off
        seg, op, a = struct.unpack_from(">HHI", out, entry)
        struct.pack_into(">I", out, entry + 4, new_addr(a))
    out += b"\0\0\0"                                    # room for the CRC
    struct.pack_into(">I", out, 0x04, len(out))
    struct.pack_into(">I", out, 0x30, new_addr(struct.unpack_from(">I", header, 0x30)[0]))
    struct.pack_into(">I", out, 0x40, idata)
    struct.pack_into(">I", out, 0x44, struct.unpack_from(">I", header, 0x44)[0] + delta)
    struct.pack_into(">H", out, 0x2E, header_parity(out))
    out[-3:] = os9_crc(out[:-3])
    open(OUT, "wb").write(out)
    for name in changed:
        print(f"MODDED: {name}")
    print(f"{os.path.relpath(OUT, ROOT)}: {len(out)} bytes ({delta:+d} bytes of code), "
          f"sha1 {hashlib.sha1(out).hexdigest()}")
    orig = path("extract", *[v.upper() for v in SUB], "modules", "cdi_zelda.mod")
    if os.path.exists(orig) and out == open(orig, "rb").read():
        print("(identical to the original)")


def main():
    if "--no-compile" not in sys.argv:
        compile_sources()
    pieces = load_pieces()
    syms = load_symbols()
    funcs = {name: start for start, _, kind, name in pieces}         # functions and assembly pieces
    funcs.update({name: v for name, (kind, v) in syms.items() if kind == "code"})
    jt, segs = load_jump_table(), load_segments()
    compiled = {}
    for o in sorted(glob.glob(path("build", *SUB, "obj", "*.o"))):
        for name, mod in mpwobj.read(o).items():
            if mod.data:
                # A global defined in C (e.g. 'long heart_x[11];'): it lives in the A5 world, at the offset
                # config/symbols.txt gives it. Defining an array in the same file as the functions that use
                # it is what makes MPW C keep its address in a3/a4. Initialized data isn't placed yet.
                if name not in syms or syms[name][0] != "data":
                    sys.exit(f"{os.path.basename(o)}: defines '{name}', which config/symbols.txt doesn't "
                             f"list as data")
                if any(mod.code):
                    sys.exit(f"{os.path.basename(o)}: '{name}' has an initializer (not supported yet)")
                continue
            if name not in funcs or name.startswith("gap_") or name in syms:
                sys.exit(f"{os.path.basename(o)}: '{name}' is not a function of the game")
            if name in compiled:
                sys.exit(f"'{name}' is defined twice")
            compiled[name] = mod
    if "--mod" in sys.argv:
        return build_mod(pieces, syms, funcs, jt, segs, compiled)
    out = bytearray(open(path("bin", *SUB, "header.bin"), "rb").read())
    failed = 0
    bad = set()                                         # compiled but not (yet) matching
    for start, end, kind, name in pieces:
        if len(out) != start:
            sys.exit(f"{name} would land at ${len(out):X} instead of ${start:X}")
        if name in compiled:
            code, errors = link_module(compiled[name], start, end - start, funcs, syms, jt, segs)
            for e in errors:
                print(f"error: {name}: {e}")
            if not errors and code != asm_bytes(name):
                print(f"DIFFERS: {name} (compiled code is not byte-identical to the game's)")
                errors = ["differs"]
            if errors:
                failed += 1
                bad.add(name)
                if "--diff" in sys.argv:
                    show_diff(name, code, asm_bytes(name), start)
                code = asm_bytes(name)
        else:
            code = asm_bytes(name)
        out += code
    out += open(path("bin", *SUB, "tail.bin"), "rb").read()
    out += os9_crc(out)
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    open(OUT, "wb").write(out)
    sha1 = hashlib.sha1(out).hexdigest()
    want = open(config("cdi_zelda.sha1")).read().split()[0]
    good = [name for name in compiled if name not in bad]
    done = sum(end - start for start, end, _, name in pieces if name in good)
    total = sum(end - start for start, end, _, _ in pieces)
    print(f"{len(good)} of {sum(1 for p in pieces if p[2] in ('func', 'fn'))} functions from C "
          f"({100 * done / total:.1f}% of the code bytes)")
    print(f"{os.path.relpath(OUT, ROOT)}: sha1 {sha1}")
    if failed or sha1 != want:
        sys.exit(f"NOT byte-identical (expected {want}); {failed} function(s) with problems")
    print("OK: byte-identical to the original")


if __name__ == "__main__":
    main()
