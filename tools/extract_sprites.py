"""Render the game's sprites to PNG: Zelda, the enemies, the items lying in rooms and the projectiles.

Usage: python3 -I tools/extract_sprites.py [--version fr|de|nl] [--only zelda|enemies|tools] [--room N]
Writes build/content/sprites/ (build/<version>/content/sprites/ for the other versions):
  zelda/frame_NN.png             Zelda's 24 frames (all cropped to one box, so they line up)
  zelda/sheet.png                the same on one sheet
  enemies/<id>_NN.png            each enemy kind's 24 frames (id from the room's enemy list, deduplicated
                                 over the rooms), enemies/<id>_sheet.png, and enemies/index.txt (which
                                 rooms each kind appears in; 69 kinds)
  tools/NN.png, tools/sheet.png  the 24 item graphics (hearts, rubies, items lying in rooms)
  projectiles/NN.png, .../sheet.png  the 48 projectile graphics (also the death puffs); a room whose
                                 tools or projectiles differ from the first ones gets a _room<r> folder
                                 (in Zelda every room has the same ones)
  Empty frames (offsets pointing at a bare rts) get no file but keep their place on the sheets.
  (--only tools does items and projectiles; --room N looks at that room only.)
The output is game data: it stays in build/, which is never published.

Where the data is (zdata.rtr, read with tools/cdi_fs.py; numbers per version in include/version.h)
  STARTUP_SECTOR ($AF in English): initial_load's record. Data channel 1 is sprite_palette, channel 4
      Zelda's imagery (zelda_imagery).
  room r (0-75) at r * $C0 + ROOM_SECTOR ($C9 in English): load_playfield's records. Record 1: channel 0 the enemy list
      (enemy ids as longs ending with -1), video channel 4 the room image; once the image is in,
      load_playfield re-points the channels (reset_pcl): channel 0 then fills room_gfx with the enemy
      graphics, one block per id of the list, in order. Record 2 (loaded with full set): channel 3 the
      projectile graphics (proj_gfx_buf, two blocks), channel 4 the tool imagery (tool_imagery_buf).

Formats
  Zelda's imagery ("PAC0"): +$0c 24 frame offsets from +$6c, then RLE frames, drawn by draw_rle_sprite
      (asm/gap_00052.s $20A) into a 72 x 56 box: per row, bytes n < $80 copy n + 1 pixels, n >= $80 skip
      n - $80 pixels, until the row's 72 pixels are used up.
  Enemy, projectile and tool graphics ("CPL0", struct enemy_gfx / struct imagery in include/game.h):
      +$04 code size, +$08 header size ($9c; the next block follows after both), +$0c 24 frame offsets
      from +$9c, +$6c 24 erase colours. Each frame is a compiled sprite: 68000 code called as a C
      function f(x, y, base, line_table) (drop_sprite, draw_projectiles, draw_tools): it adds
      line_table[y] + x to base and then stores the pixels with move.b/w/l #imm or Dn, (a0)+ and
      steps between runs with adda.l #n, a0. Lines are 416 ($1A0) bytes apart (init_table_widths),
      one byte per pixel (CLUT7).
  Here the code runs in a small 68000 interpreter (only the instructions such code needs; an unknown
  opcode stops the tool) drawing on a blank screen; pixels never written, and colour 0, are transparent.
  Colours: plane A's palette, sprite_palette: FCT instructions, a bank select ($C30000nn) and 64 longs
  ($80 + index, R, G, B) for colours 0-63, then the same for 64-127 (main: dc_wrfct of its longs 1 and $42).
"""
import argparse
import os
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
from cdi_fs import VERSIONS, Disc, version_values  # noqa: E402
from media import write_png      # noqa: E402

ROOM_SECTORS = 0xC0
ROOMS = 76                       # room_widths[76] (docs/data.md)
LINE = 0x1A0                     # bytes per screen line


# ---------------------------------------------------------------------------------------------------
# Reading the records

def read_record(disc, sector):
    """The sectors of the record starting at `sector` of zdata.rtr, up to its end-of-record sector."""
    out = []
    for _, s in disc.sectors("zdata.rtr", sector):
        out.append(s)
        if s.eor or s.eof:
            break
    return out


def channel_data(sectors, channel, after_video=None):
    """The payloads of a record's data sectors of one channel. after_video: True only those after the
    record's first video sector, False only those before (load_playfield re-points the channels when the
    room image has come in), None all."""
    buf = bytearray()
    seen_video = False
    for s in sectors:
        if s.video:
            seen_video = True
        if s.data and s.channel == channel and (after_video is None or after_video == seen_video):
            buf += s.payload
    return bytes(buf)


def sprite_palette(data):
    """128 (r, g, b) colours from sprite_palette's FCT instructions."""
    pal = [(0, 0, 0)] * 128
    bank = 0
    for k in range(0, min(len(data), 0x82 * 4), 4):
        op = data[k]
        if op == 0xC3:
            bank = data[k + 3] & 1
        elif op & 0x80:
            pal[bank * 64 + (op & 0x3F)] = tuple(data[k + 1:k + 4])
    return pal


# ---------------------------------------------------------------------------------------------------
# A small 68000 interpreter for the compiled sprites

MEM_SIZE = 0x400000
LINES_AT = 0x010000              # line_table
STACK_TOP = 0x080000
CODE_AT = 0x100000
SCREEN_AT = 0x200000
SCREEN_LINES = 512
SCREEN_END = SCREEN_AT + SCREEN_LINES * LINE
ORIGIN_X, ORIGIN_Y = 160, 200    # where on the screen a sprite is drawn (it may reach up and left of it)
RETURN = 0x0DEAD0                # the return address the call starts with
SIZES = {1: 0xFF, 2: 0xFFFF, 4: 0xFFFFFFFF}


class Unknown(Exception):
    pass


def sext(v, size):
    bits = size * 8
    v &= (1 << bits) - 1
    return v - (1 << bits) if v >> (bits - 1) else v


class CPU:
    def __init__(self):
        self.mem = bytearray(MEM_SIZE)
        for y in range(SCREEN_LINES):
            struct.pack_into(">I", self.mem, LINES_AT + y * 4, y * LINE)

    # memory
    def rd(self, addr, size):
        addr &= 0xFFFFFF
        return int.from_bytes(self.mem[addr:addr + size], "big")

    def wr(self, addr, size, v):
        addr &= 0xFFFFFF
        self.mem[addr:addr + size] = (v & SIZES[size]).to_bytes(size, "big")
        if SCREEN_AT <= addr < SCREEN_END:
            for k in range(size):
                self.touched.add(addr + k - SCREEN_AT)

    def fetch(self, size=2):
        v = self.rd(self.pc, size)
        self.pc += size
        return v

    # registers: d[0-7], a[0-7], 32 bits unsigned
    def setd(self, n, size, v):
        m = SIZES[size]
        self.d[n] = (self.d[n] & ~m & 0xFFFFFFFF) | (v & m)

    def flags(self, v, size):
        v &= SIZES[size]
        self.z = v == 0
        self.n = bool(v >> (size * 8 - 1))
        self.v = self.c = False

    # effective addresses: ('d', n), ('a', n), ('m', addr) or ('i', value)
    def ea(self, mode, reg, size):
        if mode == 0:
            return ("d", reg)
        if mode == 1:
            return ("a", reg)
        if mode == 2:
            return ("m", self.a[reg])
        if mode == 3:
            addr = self.a[reg]
            self.a[reg] = (addr + (2 if reg == 7 and size == 1 else size)) & 0xFFFFFFFF
            return ("m", addr)
        if mode == 4:
            self.a[reg] = (self.a[reg] - (2 if reg == 7 and size == 1 else size)) & 0xFFFFFFFF
            return ("m", self.a[reg])
        if mode == 5:
            return ("m", (self.a[reg] + sext(self.fetch(), 2)) & 0xFFFFFFFF)
        if mode == 6:
            return ("m", (self.a[reg] + self.index()) & 0xFFFFFFFF)
        if mode == 7:
            if reg == 0:
                return ("m", sext(self.fetch(), 2) & 0xFFFFFFFF)
            if reg == 1:
                return ("m", self.fetch(4))
            if reg == 2:
                base = self.pc
                return ("m", (base + sext(self.fetch(), 2)) & 0xFFFFFFFF)
            if reg == 3:
                base = self.pc
                return ("m", (base + self.index()) & 0xFFFFFFFF)
            if reg == 4:
                return ("i", self.fetch(4) if size == 4 else self.fetch() & SIZES[size])
        raise Unknown(f"addressing mode {mode}/{reg}")

    def index(self):
        ext = self.fetch()
        r = (ext >> 12) & 7
        x = self.a[r] if ext & 0x8000 else self.d[r]
        x = x if ext & 0x800 else sext(x, 2)
        return x + sext(ext & 0xFF, 1)

    def get(self, loc, size):
        kind, v = loc
        if kind == "d":
            return self.d[v] & SIZES[size]
        if kind == "a":
            return self.a[v] & SIZES[size]
        if kind == "m":
            return self.rd(v, size)
        return v & SIZES[size]

    def put(self, loc, size, value):
        kind, v = loc
        if kind == "d":
            self.setd(v, size, value)
        elif kind == "a":
            self.a[v] = sext(value, size) & 0xFFFFFFFF
        elif kind == "m":
            self.wr(v, size, value)
        else:
            raise Unknown("write to an immediate")

    def cond(self, cc):
        n, z, v, c = self.n, self.z, self.v, self.c
        return [True, False, not c and not z, c or z, not c, c, not z, z, not v, v, not n, n,
                n == v, n != v, not z and n == v, z or n != v][cc]

    def addsub(self, a, b, size, sub):
        """a + b or a - b with the flags."""
        m = SIZES[size]
        r = (a - b if sub else a + b) & m
        top = size * 8 - 1
        self.c = (b & m) > (a & m) if sub else (a & m) + (b & m) > m
        sa, sb, sr = a >> top & 1, b >> top & 1, r >> top & 1
        self.v = (sa != sb and sr != sa) if sub else (sa == sb and sr != sa)
        self.z, self.n = r == 0, bool(sr)
        return r

    # running
    def call(self, code, offset, args):
        """Run the routine at code[offset:] as a C function called with the long arguments args."""
        self.mem[CODE_AT:CODE_AT + len(code)] = code
        self.mem[SCREEN_AT:SCREEN_END] = bytes(SCREEN_END - SCREEN_AT)
        self.touched = set()
        self.d = [0] * 8
        self.a = [0] * 8
        self.a[7] = STACK_TOP
        self.n = self.z = self.v = self.c = False
        for v in reversed(args):
            self.a[7] -= 4
            struct.pack_into(">I", self.mem, self.a[7], v & 0xFFFFFFFF)
        self.a[7] -= 4
        struct.pack_into(">I", self.mem, self.a[7], RETURN)
        self.pc = CODE_AT + offset
        steps = 0
        while self.pc != RETURN:
            self.step()
            steps += 1
            if steps > 200000:
                raise Unknown("runs too long")

    def step(self):
        at = self.pc
        op = self.fetch()
        hi = op >> 12
        try:
            if op == 0x4E75:                                    # rts
                self.pc = self.rd(self.a[7], 4)
                self.a[7] += 4
            elif op == 0x4E71:                                  # nop
                pass
            elif hi in (1, 2, 3):                               # move / movea
                size = {1: 1, 3: 2, 2: 4}[hi]
                v = self.get(self.ea(op >> 3 & 7, op & 7, size), size)
                dmode, dreg = op >> 6 & 7, op >> 9 & 7
                if dmode == 1:
                    self.a[dreg] = sext(v, size) & 0xFFFFFFFF
                else:
                    self.put(self.ea(dmode, dreg, size), size, v)
                    self.flags(v, size)
            elif op & 0xF100 == 0x7000:                         # moveq
                v = sext(op & 0xFF, 1)
                self.d[op >> 9 & 7] = v & 0xFFFFFFFF
                self.flags(v, 4)
            elif op & 0xFFB8 == 0x4880:                         # ext.w / ext.l
                r = op & 7
                if op & 0x40:
                    self.d[r] = sext(self.d[r], 2) & 0xFFFFFFFF
                else:
                    self.setd(r, 2, sext(self.d[r], 1))
            elif op & 0xFFF8 == 0x4840:                         # swap
                r = op & 7
                self.d[r] = (self.d[r] >> 16 | self.d[r] << 16) & 0xFFFFFFFF
            elif op & 0xFB80 == 0x4880:                         # movem
                self.movem(op)
            elif op & 0xF1C0 == 0x41C0:                         # lea
                self.a[op >> 9 & 7] = self.ea(op >> 3 & 7, op & 7, 4)[1]
            elif op & 0xFF00 in (0x4200, 0x4A00) and op & 0xC0 != 0xC0:   # clr / tst
                size = 1 << (op >> 6 & 3)
                loc = self.ea(op >> 3 & 7, op & 7, size)
                if op & 0xFF00 == 0x4200:
                    self.put(loc, size, 0)
                    self.flags(0, size)
                else:
                    self.flags(self.get(loc, size), size)
            elif op & 0xF0C0 in (0xD0C0, 0x90C0):               # adda / suba
                size = 4 if op & 0x100 else 2
                v = sext(self.get(self.ea(op >> 3 & 7, op & 7, size), size), size)
                r = op >> 9 & 7
                self.a[r] = (self.a[r] + (v if hi == 0xD else -v)) & 0xFFFFFFFF
            elif op & 0xF1C0 == 0xC0C0:                         # mulu.w
                v = self.get(self.ea(op >> 3 & 7, op & 7, 2), 2)
                r = op >> 9 & 7
                self.d[r] = (self.d[r] & 0xFFFF) * v
                self.flags(self.d[r], 4)
            elif hi in (0x8, 0x9, 0xB, 0xC, 0xD) and op & 0xC0 != 0xC0:   # or, sub, cmp/eor, and, add
                self.alu(op)
            elif hi == 0xB and op & 0xC0 == 0xC0:               # cmpa
                size = 4 if op & 0x100 else 2
                v = sext(self.get(self.ea(op >> 3 & 7, op & 7, size), size), size)
                self.addsub(self.a[op >> 9 & 7], v & 0xFFFFFFFF, 4, True)
            elif hi == 0xE and op & 0xC0 != 0xC0:               # register shifts
                self.shift(op)
            elif hi == 5 and op & 0xF8 == 0xC8:                 # dbcc
                disp = sext(self.fetch(), 2)
                if not self.cond(op >> 8 & 0xF):
                    r = op & 7
                    cnt = (self.d[r] - 1) & 0xFFFF
                    self.setd(r, 2, cnt)
                    if cnt != 0xFFFF:
                        self.pc = at + 2 + disp
            elif hi == 5 and op & 0xC0 != 0xC0:                 # addq / subq
                size = 1 << (op >> 6 & 3)
                q = (op >> 9 & 7) or 8
                mode, reg = op >> 3 & 7, op & 7
                if mode == 1:
                    self.a[reg] = (self.a[reg] + (-q if op & 0x100 else q)) & 0xFFFFFFFF
                else:
                    loc = self.ea(mode, reg, size)
                    self.put(loc, size, self.addsub(self.get(loc, size), q, size, bool(op & 0x100)))
            elif hi == 6:                                       # bra / bsr / bcc
                disp = sext(op & 0xFF, 1)
                if disp == 0:
                    disp = sext(self.fetch(), 2)
                cc = op >> 8 & 0xF
                if cc == 1:
                    self.a[7] -= 4
                    struct.pack_into(">I", self.mem, self.a[7], self.pc)
                    self.pc = at + 2 + disp
                elif self.cond(cc):
                    self.pc = at + 2 + disp
            elif op & 0xF100 == 0 and op & 0xC0 != 0xC0 and (op >> 9 & 7) in (0, 1, 2, 3, 5, 6):
                self.immediate(op)                              # ori, andi, subi, addi, eori, cmpi
            elif op & 0xFF00 == 0x0800:                         # btst/bchg/bclr/bset #n
                self.bitop(op, self.fetch() & 0xFF)
            elif op & 0xF100 == 0x0100 and op >> 3 & 7 != 1:    # btst/bchg/bclr/bset Dn
                self.bitop(op, self.d[op >> 9 & 7])
            else:
                raise Unknown("opcode")
        except Unknown as e:
            raise Unknown(f"{e}: {op:04x} at code +{at - CODE_AT:#x}") from None

    def movem(self, op):
        size = 4 if op & 0x40 else 2
        mask = self.fetch()
        mode, reg = op >> 3 & 7, op & 7
        regs = [("d", k) for k in range(8)] + [("a", k) for k in range(8)]
        if op & 0x400:                                          # memory to registers
            addr = self.ea(mode, reg, size)[1] if mode != 3 else self.a[reg]
            for k in range(16):
                if mask >> k & 1:
                    v = sext(self.rd(addr, size), size) & 0xFFFFFFFF
                    getattr(self, regs[k][0])[regs[k][1]] = v
                    addr += size
            if mode == 3:
                self.a[reg] = addr
        elif mode == 4:                                         # registers to -(An): mask reversed
            addr = self.a[reg]
            for k in range(16):
                if mask >> k & 1:
                    kind, n = regs[15 - k]
                    addr -= size
                    self.wr(addr, size, getattr(self, kind)[n])
            self.a[reg] = addr
        else:
            addr = self.ea(mode, reg, size)[1]
            for k in range(16):
                if mask >> k & 1:
                    kind, n = regs[k]
                    self.wr(addr, size, getattr(self, kind)[n])
                    addr += size

    def alu(self, op):
        hi = op >> 12
        size = 1 << (op >> 6 & 3)
        r = op >> 9 & 7
        to_ea = bool(op & 0x100)
        if hi == 0xB and to_ea:                                 # eor Dn, <ea>
            loc = self.ea(op >> 3 & 7, op & 7, size)
            v = self.get(loc, size) ^ self.d[r]
            self.put(loc, size, v)
            self.flags(v, size)
            return
        if to_ea and op >> 3 & 7 in (0, 1):
            raise Unknown("addx/subx/abcd/sbcd")
        loc = self.ea(op >> 3 & 7, op & 7, size)
        a, b = (self.get(loc, size), self.d[r] & SIZES[size]) if to_ea else (self.d[r] & SIZES[size], self.get(loc, size))
        if hi == 0x8:
            v = a | b
            self.flags(v, size)
        elif hi == 0xC:
            v = a & b
            self.flags(v, size)
        elif hi == 0xD:
            v = self.addsub(a, b, size, False)
        else:
            v = self.addsub(a, b, size, True)
        if hi == 0xB:                                           # cmp: flags only
            return
        if to_ea:
            self.put(loc, size, v)
        else:
            self.setd(r, size, v)

    def immediate(self, op):
        kind = op >> 9 & 7
        size = 1 << (op >> 6 & 3)
        imm = self.fetch(4) if size == 4 else self.fetch() & SIZES[size]
        loc = self.ea(op >> 3 & 7, op & 7, size)
        a = self.get(loc, size)
        if kind == 0:
            v = a | imm
        elif kind == 1:
            v = a & imm
        elif kind == 5:
            v = a ^ imm
        elif kind in (2, 6):
            v = self.addsub(a, imm, size, kind == 2)
        else:                                                   # cmpi
            self.addsub(a, imm, size, True)
            return
        if kind in (0, 1, 5):
            self.flags(v, size)
        self.put(loc, size, v)

    def bitop(self, op, bit):
        mode, reg = op >> 3 & 7, op & 7
        size = 4 if mode == 0 else 1
        bit &= size * 8 - 1
        loc = self.ea(mode, reg, size)
        v = self.get(loc, size)
        self.z = not v >> bit & 1
        kind = op >> 6 & 3
        if kind:
            v = [v, v ^ (1 << bit), v & ~(1 << bit), v | (1 << bit)][kind]
            self.put(loc, size, v)

    def shift(self, op):
        size = 1 << (op >> 6 & 3)
        r = op & 7
        count = self.d[op >> 9 & 7] & 63 if op & 0x20 else (op >> 9 & 7) or 8
        kind, left = op >> 3 & 3, bool(op & 0x100)
        bits = size * 8
        m = SIZES[size]
        v = self.d[r] & m
        if kind in (0, 1):                                      # asl/asr, lsl/lsr
            if left:
                v = (v << count) & m
            elif kind == 0:
                v = (sext(v, size) >> count) & m
            else:
                v >>= count
        elif kind == 3:                                         # rol/ror
            count %= bits
            v = ((v << count) | (v >> (bits - count))) & m if left else ((v >> count) | (v << (bits - count))) & m
        else:
            raise Unknown("roxl/roxr")
        self.setd(r, size, v)
        self.flags(v, size)


def render_compiled(cpu, block, frame):
    """The pixels a compiled sprite frame draws: {(x, y): colour} relative to its position."""
    offset = struct.unpack_from(">I", block, 0x0C + frame * 4)[0]
    cpu.call(block, 0x9C + offset, [ORIGIN_X, ORIGIN_Y, SCREEN_AT, LINES_AT])
    out = {}
    for k in cpu.touched:
        y, x = divmod(k, LINE)
        out[(x - ORIGIN_X, y - ORIGIN_Y)] = cpu.mem[SCREEN_AT + k]
    return out


def render_rle(data, frame):
    """A frame of Zelda's imagery, as draw_rle_sprite draws it (72 x 56 pixels)."""
    p = 0x6C + struct.unpack_from(">I", data, 0x0C + frame * 4)[0]
    out = {}
    for y in range(56):
        x = 0
        left = 0x47                         # d1: the row's pixel count, a dbra counter changed with sub.b
        while True:
            n = data[p]
            p += 1
            if n < 0x80:
                left = (left & 0xFF00) | ((left - n) & 0xFF)
                for _ in range(n + 1):
                    out[(x, y)] = data[p]
                    x += 1
                    p += 1
            else:
                n &= 0x7F
                x += n
                left = (left & 0xFF00) | ((left - n + 1) & 0xFF)
            left = (left - 1) & 0xFFFF
            if left == 0xFFFF:
                break
    return out


# ---------------------------------------------------------------------------------------------------
# Writing

def bbox(frames):
    pts = [p for f in frames for p, c in f.items() if c]
    if not pts:
        return 0, 0, 1, 1
    xs, ys = [p[0] for p in pts], [p[1] for p in pts]
    return min(xs), min(ys), max(xs) + 1, max(ys) + 1


def save(path, frame, box, palette):
    x0, y0, x1, y1 = box
    w, h = x1 - x0, y1 - y0
    pix = bytearray(w * h)
    for (x, y), c in frame.items():
        if x0 <= x < x1 and y0 <= y < y1:
            pix[(y - y0) * w + x - x0] = c
    write_png(path, w, h, pix, (palette, 0))


def save_sheet(path, frames, box, palette, cols=6):
    """All frames in a grid, one cell per frame, with a 2-pixel gap."""
    x0, y0, x1, y1 = box
    w, h = x1 - x0, y1 - y0
    while len(frames) > 1 and not any(frames[-1].values()):    # leave out trailing empty frames
        frames = frames[:-1]
    rows = -(-len(frames) // cols)
    W, H = cols * (w + 2), rows * (h + 2)
    pix = bytearray(W * H)
    for i, f in enumerate(frames):
        ox, oy = (i % cols) * (w + 2), (i // cols) * (h + 2)
        for (x, y), c in f.items():
            if x0 <= x < x1 and y0 <= y < y1:
                pix[(oy + y - y0) * W + ox + x - x0] = c
    write_png(path, W, H, pix, (palette, 0))


def write_set(out_dir, name, frames, palette, together, sheet="sheet"):
    """Write frames as <name>NN.png, cropped all to one box (together: they line up as an animation) or
    each to its own, skipping empty frames (frame offsets that point at a bare rts), and <sheet>.png
    with all of them in a grid, in frame order."""
    os.makedirs(out_dir, exist_ok=True)
    box = bbox(frames)
    for i, f in enumerate(frames):
        if any(f.values()):
            save(os.path.join(out_dir, f"{name}{i:02}.png"), f, box if together else bbox([f]), palette)
    save_sheet(os.path.join(out_dir, f"{sheet}.png"), frames, box, palette)


def blocks(data, count):
    """count "CPL0" blocks one after the other (each f4 + f8 bytes)."""
    out, p = [], 0
    for _ in range(count):
        if data[p:p + 4] != b"CPL0":
            raise ValueError(f"no compiled-sprite block at +{p:#x}")
        size = sum(struct.unpack_from(">II", data, p + 4))
        out.append(data[p:p + size])
        p += size
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--version", default="en", choices=VERSIONS)
    ap.add_argument("--only", choices=["zelda", "enemies", "tools"])
    ap.add_argument("--room", type=int)
    args = ap.parse_args()
    values = version_values(args.version)         # sector numbers, from include/version.h
    startup, room_sector = values["STARTUP_SECTOR"], values["ROOM_SECTOR"]
    disc = Disc.open(args.version)
    out = os.path.join(ROOT, "build", *([] if args.version == "en" else [args.version]), "content", "sprites")
    want = {args.only} if args.only else {"zelda", "enemies", "tools"}
    cpu = CPU()

    rec = read_record(disc, startup)
    palette = sprite_palette(channel_data(rec, 1))
    palette = palette + palette                         # indices 128-255 (should not occur) wrap around

    if "zelda" in want:
        zelda = channel_data(rec, 4)
        assert zelda[:4] == b"PAC0", "Zelda's imagery not found"
        frames = [render_rle(zelda, i) for i in range(24)]
        write_set(os.path.join(out, "zelda"), "frame_", frames, palette, True)
        print("zelda: 24 frames")

    if not want & {"enemies", "tools"}:
        return
    rooms = [args.room] if args.room is not None else range(ROOMS)
    seen = {}                                           # enemy id -> rooms
    tools_done = set()
    for r in rooms:
        sector = r * ROOM_SECTORS + room_sector
        if sector >= disc.files["zdata.rtr"][1] // 2048:
            break
        rec1 = read_record(disc, sector)
        ids_data = channel_data(rec1, 0, after_video=False)
        gfx = channel_data(rec1, 0, after_video=True)
        if not ids_data or gfx[:4] != b"CPL0":
            continue
        ids = []
        for k in range(0, 16, 4):
            v = struct.unpack_from(">i", ids_data, k)[0]
            if v == -1:
                break
            ids.append(v)
        if "enemies" in want:
            for eid, block in zip(ids, blocks(gfx, len(ids))):
                seen.setdefault(eid, []).append(r)
                if len(seen[eid]) > 1:
                    continue
                frames = [render_compiled(cpu, block, i) for i in range(24)]
                write_set(os.path.join(out, "enemies"), f"{eid}_", frames, palette, True, f"{eid}_sheet")
                print(f"room {r}: enemy {eid}")
        if "tools" in want:
            rec2 = read_record(disc, sector + len(rec1))
            proj, tool = channel_data(rec2, 3), channel_data(rec2, 4)
            for name, data, n in (("projectiles", proj, 2), ("tools", tool, 1)):
                if data[:4] != b"CPL0":
                    continue
                key = (name, bytes(data[:sum(len(b) for b in blocks(data, n))]))
                if key in tools_done:
                    continue
                suffix = "" if not any(k[0] == name for k in tools_done) else f"_room{r}"
                tools_done.add(key)
                frames = [render_compiled(cpu, b, i) for b in blocks(data, n) for i in range(24)]
                write_set(os.path.join(out, name + suffix), "", frames, palette, False)
                print(f"room {r}: {name}{suffix} ({len(frames)} frames)")
    if "enemies" in want and seen:
        with open(os.path.join(out, "enemies", "index.txt"), "w") as f:
            for eid in sorted(seen):
                f.write(f"{eid:4}  rooms {' '.join(map(str, seen[eid]))}\n")


if __name__ == "__main__":
    main()
