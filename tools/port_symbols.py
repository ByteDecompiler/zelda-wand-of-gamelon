"""Find the A5 offsets of the globals in another version of the game (French, German or Dutch).

Usage (from the project root, after an English build and `tools/split.py --version <v>`):
  python3 tools/port_symbols.py [--version fr|de|nl]

Other versions were built from (nearly) the same C, but their globals can sit elsewhere in the A5 world.
For every compiled function (build/obj/) this reads, at the function's place in the other version's
module, each reference the object file names (a global or a jump-table entry), and so learns that
symbol's offset there. Every reading of a symbol must agree. Symbols that no C function refers to get
the shift of their nearest neighbour below them (marked "guessed"). Jump-table entries no code reads
are found through the jump table, and routines inside assembly pieces by their place in the piece.
Writes config/<version>/symbols.txt: only the symbols whose offset differs from config/symbols.txt.
"""
import glob
import os
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import build  # noqa: E402
import mpwobj  # noqa: E402


def pieces(config):
    out = {}
    for line in open(os.path.join(config, "functions.txt")):
        if line.strip() and not line.startswith("#"):
            start, end, kind, name = line.split()
            out[name] = (int(start, 16), int(end, 16))
    return out


def jump_table(config):
    """A5 offset of each jump-table entry -> (segment, module address)."""
    out = {}
    for line in open(os.path.join(config, "jumptable.txt")):
        if not line.startswith("#"):
            off, seg, addr = line.split()
            out[int(off, 16)] = (seg, int(addr, 16))
    return out


def by_jump_table(off, en_config, config):
    """The other version's jump-table offset of the routine at English offset off: a game function by its
    name, a library routine (segment 9) by its place in the library, which is the same code in every
    version."""
    ej, oj = jump_table(en_config), jump_table(config)
    if off not in ej:
        return None
    seg, addr = ej[off]
    if seg == "9":
        lib = lambda t: min(a for s, a in t.values() if s == "9")
        target = addr - lib(ej) + lib(oj)
    else:
        name = {a: n for n, (a, _) in pieces(en_config).items()}.get(addr)
        target = pieces(config).get(name, (None,))[0]
    return next((o for o, (_, a) in oj.items() if a == target), None)


def by_piece(addr, en_config, config):
    """The other version's address of a routine inside an assembly piece: the same offset in the piece at
    the same position (the gaps keep their order)."""
    def gaps(c):
        return sorted(a for a, b in [v for n, v in pieces(c).items() if n.startswith("gap_")])
    eg, og = gaps(en_config), gaps(config)
    i = max(k for k, a in enumerate(eg) if a <= addr)
    return og[i] + addr - eg[i]


def main():
    version = sys.argv[sys.argv.index("--version") + 1] if "--version" in sys.argv else "fr"
    config = os.path.join(ROOT, "config", version)
    en_mod = open(os.path.join(ROOT, "extract", "modules", "cdi_zelda.mod"), "rb").read()
    mod = open(os.path.join(ROOT, "extract", version.upper(), "modules", "cdi_zelda.mod"), "rb").read()
    build.SUB = []                                      # the English symbols
    syms = build.load_symbols()
    en, other = pieces(os.path.join(ROOT, "config")), pieces(config)

    seen = {}                                           # symbol -> {offset in the other version: [functions]}
    for o in sorted(glob.glob(os.path.join(ROOT, "build", "obj", "*.o"))):
        for name, m in mpwobj.read(o).items():
            if m.data or name not in en or name not in other:
                continue
            (es, ee), (os_, oe) = en[name], other[name]
            if ee - es != oe - os_:
                print(f"{name}: a different size in this version, skipped")
                continue
            for off, sym, a5 in m.refs:
                if not a5 or sym not in syms or syms[sym][0] == "code":
                    continue
                field = int.from_bytes(m.code[off:off + 2], "big", signed=True)
                if struct.unpack_from(">h", en_mod, es + off)[0] != syms[sym][1] + field:
                    continue                            # doesn't match the English code: can't be read
                value = struct.unpack_from(">h", mod, os_ + off)[0] - field
                seen.setdefault(sym, {}).setdefault(value, []).append(name)

    rows = [f"# A5 offsets of the globals that moved in the {version.upper()} version (written by "
            f"tools/port_symbols.py; the others are as in config/symbols.txt).", ""]
    conflicts = 0
    found = {}
    for sym, values in seen.items():
        if len(values) > 1:
            conflicts += 1
            print(f"conflict: {sym}: " + ", ".join(f"{v:#x} ({', '.join(f)})" for v, f in values.items()))
        found[sym] = max(values, key=lambda v: len(values[v]))
    data = sorted((v, s) for s, (k, v) in syms.items() if k == "data")
    moved = guessed = 0
    for kind in ("data", "jt", "code"):
        for name, (k, v) in syms.items():
            if k != kind:
                continue
            if kind == "code":
                new, note = by_piece(v, os.path.join(ROOT, "config"), config), ""
            elif kind == "jt":
                new, note = by_jump_table(v, os.path.join(ROOT, "config"), config), ""
                if new is None:
                    new = found.get(name)
                if new is None:
                    continue
            elif name in found:
                new, note = found[name], ""
            elif kind == "data":
                # the shift of the nearest symbol below (lower offset) whose place is known
                below = [s for off, s in data if off <= v and s in found]
                if not below:
                    continue
                new, note = v + found[below[-1]] - syms[below[-1]][1], "   # guessed"
                guessed += new != v
            if new != v:
                moved += 1
                rows.append(f"{kind} {name:<24} {'-' if new < 0 else ''}{abs(new):#x}{note}")
    os.makedirs(config, exist_ok=True)
    open(os.path.join(config, "symbols.txt"), "w", newline="\n").write("\n".join(rows) + "\n")
    print(f"{len(found)} symbols read, {moved} moved ({guessed} guessed), {conflicts} conflict(s); "
          f"wrote config/{version}/symbols.txt")


if __name__ == "__main__":
    main()
