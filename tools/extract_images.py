"""Extract the game's static images and palettes from the disc image as PNG files.

Usage: python3 -I tools/extract_images.py [--version fr|de|nl] [--only rooms|map|startup|palettes|lanim] [--room N]
  --only palettes writes just the palette swatches (of everything); --room N extracts that room only.

Writes build/content/images/ (build/<version>/content/images/ for the other versions). The output is game data: it
stays in build/, which is never published. Compiled sprites (enemies, Zelda, items: 68000 code) are not
images and are left to tools/extract_sprites.py.

  rooms/room_NN.png          each room's background (plane B), with the room's palette
  worldmap.png               the world map; labels/label_NN.png the place names drawn over it
  startup/item_NN.png        the 24 item icons (item 23 is blank), startup/menu_NN.png the 24 menu icons
                             (the subscreen's frame pieces, swords and shields; plane A)
  startup/font.png           the font's glyphs (characters $20-$7F, 16 a row)
  lanim/piracy_ntsc.png, lanim/piracy_pal.png    the anti-piracy notice shown after the credits
  lanim/background_RR_F.png  the backgrounds behind the full-screen animations (record RR, frame F: the
                             frame at sector F * 7 of the record's data; only the frames the game shows)
  palettes/*.png             each palette as a 16 x 8 grid of swatches (index 0 top left)

How the data is found
---------------------
Each load is an ss_play of one record of a real-time file: the loader's PCL lists (set_pcl) send each
channel's sectors into a buffer, data channels and video channels separately. play() below does the same
demultiplexing from the sector subheaders, up to the end-of-record sector.

zdata.rtr sector 0 (load_world): data channel 0 = palette file; video channel 0 = the map image
  (plane_b_buf0); video channel 1 = the label images (room_gfx).
zdata.rtr sector STARTUP_SECTOR (initial_load, include/version.h): data channel 0 = plane A CLUT used by
  the animations (clut_table), 1 = plane A CLUT of the game (sprite_palette, installed by main), 2 = menu
  icons and font (menu_icons_buf), 3 = macro data, 4 = Zelda's imagery, 5 = collision maps.
zdata.rtr sector room * $C0 + ROOM_SECTOR (load_playfield): data channel 4 = the palette file, video
  channel 4 = the room image (plane_b_buf0); the other channels are enemy data and compiled graphics.
zanim.rtr record $3D (NTSC) / $41 (PAL), at sector (record - 1) * 1500 (load_piracy): data channel 1 =
  palette file, video channel 1 = a 384 x 240 image.
zanim.rtr record 57 (the intro, 56 sectors loaded) and 58 (the ending, 49 sectors) (load_backgrounds):
  data channel 1 = RL7 backgrounds, at the sector offsets listed in bg_frames_b (intro) and bg_frames_a
  (ending), 15 entries each (multiples of 7), each shown by pointing line 0 of plane B's LCT at it
  (flip_backgrounds). The colours are assumed to be the animation CLUT (clut_table), which
  play_background_animation loads into both planes. load_backgrounds can also load record 60 into a
  video channel (its third case), but nothing calls it that way, so it isn't extracted.

Formats
-------
Video sectors are Form 2 with coding 1 (CLUT7): one byte a pixel, a 7-bit palette index, the 2324-byte
payloads following on from each other.
- Palette file (load_world, load_playfield, roll_credits): a 4-byte header (00 00 00 80), then 128 RGB
  triples (convert_palette -> install_palette loads colours 0-63 and 64-127 into the plane's CLUT).
- CLUT load commands (clut_table, sprite_palette: $82 longs each, copied by initial_load): $C30000bb
  selects bank bb (64 colours each), then $80+i rr gg bb sets colour i of the bank. The plane's colours
  0-63 are the first bank listed, 64-127 the second (install_palette, post_bumper_video_init).
- Room image: line i starts at plane_b_buf0 + i * line_step with line_step = room_widths[r] * 384 and
  room_heights[r] * 240 lines (init_playfield_size, reinit_lct_B): a plain row-major image of the whole
  room, however many screens wide or high.
- Rooms: 76 (room_widths[76]); labels: 15 (labels[15], label_slots[15]).
- World map: 704 pixels a line (move_map: the LCT steps 704 bytes a line); scroll_world keeps map_y
  <= $C8, so 240 + $C8 = 440 lines are ever shown (the stream holds a few more bytes).
- Labels (show_label, draw_label_save_under): label i is labels[i].w x labels[i].h bytes, row-major,
  at label_slots[i] * $914 in the label stream (one or more 2324-byte video sectors each).
- Icons (draw_rle_icon): 72 x 56; per line, bytes: n < $80 copies the next n + 1 pixels, n >= $80 skips
  n & $7F (transparent) pixels. The icon data starts with a header (struct imagery): 24 offsets at +$0C,
  counted from +$6C (load_menu_icons: item icons at +0, menu icons at +$1FD2).
- Font (load_menu_icons, load_font, font_draw_glyphs): at +$32CE of the menu data, 96 glyph offsets
  (words, for characters $20-$7F), 96 widths, then the glyphs; a glyph is 27 lines of its width, bytes
  >= 0 one pixel set (drawn in the text colour), bytes with bit 7 skip n & $7F pixels.
- RL7 (the animation backgrounds): see rl7() below.

The game's globals (room sizes, labels, label slots, background frame offsets) come from the initial values in the program
module cdi_zelda on the disc, unpacked the way tools/a5init.py does.
"""
import os
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import build      # noqa: E402  (its --version option picks the version's symbols)
import a5init     # noqa: E402
from cdi_fs import VERSIONS, Disc, version_values  # noqa: E402
from media import write_png     # noqa: E402

ROOMS = 76                      # room_widths[76], room_heights[76]
LABELS = 15                     # labels[15], label_slots[15]
SCREEN_W, SCREEN_H = 384, 240
MAP_W, MAP_H = 704, 240 + 0xc8
TRANSPARENT = 128               # the extra palette entry used for transparent pixels in icons and the font


def play(disc, name, sector):
    """The record at `sector` of real-time file `name`, demultiplexed: {("video" or "data", channel):
    bytes}, each channel's payloads in disc order, up to the end-of-record sector."""
    out = {}
    for _, s in disc.sectors(name, sector):
        kind = "video" if s.video else "data" if s.data else None
        if kind:
            out.setdefault((kind, s.channel), bytearray()).extend(s.payload)
        if s.eor:
            break
    return out


class Globals:
    """Initial values of the game's globals, from the program module on the disc."""

    def __init__(self, disc):
        size = disc.files["cdi_zelda"][1]
        mod = b"".join(s.payload for _, s in disc.sectors("cdi_zelda"))[:size]
        self.img, _ = a5init.unpack(mod)
        self.below = a5init.BELOW
        self.syms = {n: v for n, (kind, v) in build.load_symbols().items() if kind == "data"}

    def bytes(self, name, n):
        p = self.below + self.syms[name]          # data symbols are negative A5 offsets
        return bytes(self.img[p:p + n])


def file_palette(data):
    """A palette file: 4-byte header, then 128 RGB triples."""
    return [tuple(data[4 + i * 3:7 + i * 3]) for i in range(128)]


def clut_palette(data):
    """$82 longs of CLUT load commands: a bank select, 64 colours, a bank select, 64 colours."""
    pal = [(0, 0, 0)] * 128
    half = -1
    for k in range(0x82):
        cmd = struct.unpack_from(">I", data, k * 4)[0]
        if cmd >> 24 == 0xc3:
            half += 1
        elif 0x80 <= cmd >> 24 < 0xc0 and 0 <= half < 2:
            pal[half * 64 + (cmd >> 24) - 0x80] = ((cmd >> 16) & 0xff, (cmd >> 8) & 0xff, cmd & 0xff)
    return pal


palettes_only = False          # --only palettes: write the palette swatches and skip the images


def save(path, w, h, pixels, pal, transparent=False):
    if palettes_only and os.path.basename(os.path.dirname(path)) != "palettes":
        return
    os.makedirs(os.path.dirname(path), exist_ok=True)
    if transparent:
        write_png(path, w, h, pixels, (list(pal) + [(0, 0, 0)], TRANSPARENT))
    else:
        write_png(path, w, h, pixels, pal)


def save_swatches(path, pal):
    """The palette as 16 x 8 squares of 16 pixels, colour 0 top left, row by row."""
    w, h = 16 * 16, 8 * 16
    px = bytes((y // 16) * 16 + x // 16 for y in range(h) for x in range(w))
    save(path, w, h, px, pal)


def clut7(data, w, h):
    """w x h pixels of a CLUT7 image (padded with colour 0 if the stream is short)."""
    px = bytes(b & 0x7f for b in data[:w * h])
    return px + bytes(w * h - len(px))


def rl7(data, p, w, h):
    """w x h pixels of a CLUT7 run-length image (RL7, the CD-i format set_fctB_rl7 switches plane B to) at
    data[p]: a byte with bit 7 clear is one pixel of that colour; with bit 7 set, the colour (low 7 bits)
    is followed by a run length, 0 meaning to the end of the line."""
    px = bytearray()
    for _ in range(h):
        line = bytearray()
        while len(line) < w:
            b = data[p]
            p += 1
            if b & 0x80:
                n = data[p]
                p += 1
                line += bytes([b & 0x7f]) * (w - len(line) if n == 0 else n)
            else:
                line.append(b)
        px += line[:w]
    return bytes(px)


def decode_icon(data, p):
    """A 72 x 56 RLE icon at data[p] (draw_rle_icon)."""
    w, h = 72, 56
    px = bytearray([TRANSPARENT]) * (w * h)
    for y in range(h):
        x = 0
        while x < w:
            n = data[p]
            p += 1
            if n & 0x80:
                x += n & 0x7f
            else:
                for _ in range(n + 1):
                    if x < w:
                        px[y * w + x] = data[p] & 0x7f
                    x += 1
                    p += 1
    return px


def decode_glyph(data, p, w, h, colour):
    """A font glyph (font_draw_glyphs): h lines of w pixels."""
    px = bytearray([TRANSPARENT]) * (w * h)
    for y in range(h):
        x = 0
        while x < w:
            n = data[p]
            p += 1
            if n & 0x80:
                x += n & 0x7f
            else:
                px[y * w + x] = colour
                x += 1
    return px


def extract_rooms(disc, g, out, sectors, only_room=None):
    widths = g.bytes("room_widths", ROOMS)
    heights = g.bytes("room_heights", ROOMS)
    for r in range(ROOMS):
        if only_room is not None and r != only_room:
            continue
        if not widths[r] or not heights[r]:
            print(f"  room {r:2}: unused (size 0)")
            continue
        rec = play(disc, "zdata.rtr", r * 0xc0 + sectors["ROOM_SECTOR"])
        image, pal_file = rec.get(("video", 4)), rec.get(("data", 4))
        if not image or not pal_file:
            print(f"  room {r}: no image")
            continue
        w, h = widths[r] * SCREEN_W, heights[r] * SCREEN_H
        pal = file_palette(pal_file)
        short = "" if len(image) >= w * h else f" (stream short by {w * h - len(image)} bytes)"
        save(os.path.join(out, "rooms", f"room_{r:02}.png"), w, h, clut7(image, w, h), pal)
        save_swatches(os.path.join(out, "palettes", f"room_{r:02}.png"), pal)
        print(f"  room {r:2}: {widths[r]} x {heights[r]} screens{short}")


def extract_map(disc, g, out):
    rec = play(disc, "zdata.rtr", 0)
    pal = file_palette(rec[("data", 0)])
    save_swatches(os.path.join(out, "palettes", "worldmap.png"), pal)
    save(os.path.join(out, "worldmap.png"), MAP_W, MAP_H, clut7(rec[("video", 0)], MAP_W, MAP_H), pal)
    labels = g.bytes("labels", LABELS * 18)
    slots = struct.unpack(f">{LABELS}I", g.bytes("label_slots", LABELS * 4))
    stream = rec[("video", 1)]
    for i in range(LABELS):
        x, y, w, h = struct.unpack_from(">4H", labels, i * 18)
        if not w or not h:
            continue
        p = slots[i] * 0x914
        save(os.path.join(out, "labels", f"label_{i:02}.png"), w, h, clut7(stream[p:], w, h), pal)
    print(f"  world map {MAP_W} x {MAP_H}, {LABELS} labels")


def extract_startup(disc, out, sectors):
    rec = play(disc, "zdata.rtr", sectors["STARTUP_SECTOR"])
    anim_pal = clut_palette(rec[("data", 0)])
    game_pal = clut_palette(rec[("data", 1)])
    save_swatches(os.path.join(out, "palettes", "plane_a_anim.png"), anim_pal)
    save_swatches(os.path.join(out, "palettes", "plane_a_game.png"), game_pal)
    data = rec[("data", 2)]
    for prefix, base in (("item", 0), ("menu", 0x1fd2)):
        for i in range(24):
            off = struct.unpack_from(">I", data, base + 0x0c + i * 4)[0]
            save(os.path.join(out, "startup", f"{prefix}_{i:02}.png"), 72, 56,
                 decode_icon(data, base + 0x6c + off), game_pal, transparent=True)
    # the font: 16 characters a row, each in a cell as wide as the widest glyph
    font, height = 0x32ce, 27
    offs = struct.unpack_from(">96H", data, font)
    widths = struct.unpack_from(">96H", data, font + 0xc0)
    cw, ch = max(widths) + 2, height + 2
    sheet = bytearray([TRANSPARENT]) * (16 * cw * 6 * ch)
    white = max(range(128), key=lambda i: sum(game_pal[i]))
    for c in range(96):
        glyph = decode_glyph(data, font + 0x180 + offs[c], widths[c], height, white)
        x0, y0 = (c % 16) * cw + 1, (c // 16) * ch + 1
        for y in range(height):
            row = (y0 + y) * 16 * cw + x0
            sheet[row:row + widths[c]] = glyph[y * widths[c]:(y + 1) * widths[c]]
    save(os.path.join(out, "startup", "font.png"), 16 * cw, 6 * ch, sheet, game_pal, transparent=True)
    print("  24 item icons, 24 menu icons, font, 2 plane A palettes")


def extract_lanim(disc, g, out, sectors):
    # the backgrounds behind the full-screen animations (load_backgrounds, play_background_animation):
    # RL7 frames at the sector offsets of bg_frames_b (the intro, record 57) and bg_frames_a (the ending,
    # record 58), shown with the animation CLUT (clut_table, on both planes)
    anim_pal = clut_palette(play(disc, "zdata.rtr", sectors["STARTUP_SECTOR"])[("data", 0)])
    count = 0
    for record, size, table in ((57, 56, "bg_frames_b"), (58, 49, "bg_frames_a")):
        data = play(disc, "zanim.rtr", (record - 1) * 1500)[("data", 1)][:size * 0x800]
        offsets = sorted(set(struct.unpack(">15I", g.bytes(table, 15 * 4))))
        for off in offsets:
            save(os.path.join(out, "lanim", f"background_{record}_{off // 7}.png"), SCREEN_W, SCREEN_H,
                 rl7(data, off * 0x800, SCREEN_W, SCREEN_H), anim_pal)
            count += 1
    for name, record in (("ntsc", 0x3d), ("pal", 0x41)):
        rec = play(disc, "zanim.rtr", (record - 1) * 1500)
        pal = file_palette(rec[("data", 1)])
        save(os.path.join(out, "lanim", f"piracy_{name}.png"), SCREEN_W, SCREEN_H,
             clut7(rec[("video", 1)], SCREEN_W, SCREEN_H), pal)
        save_swatches(os.path.join(out, "palettes", f"piracy_{name}.png"), pal)
    print(f"  {count} animation backgrounds, 2 piracy notice images")


def main():
    args = sys.argv[1:]

    def option(name):
        return args[args.index(name) + 1] if name in args else None

    version = option("--version") or "en"
    only = option("--only")
    room = option("--room")
    room = int(room, 0) if room is not None else None
    if room is not None and not only:
        only = "rooms"
    if only not in (None, "rooms", "map", "startup", "palettes", "lanim"):
        sys.exit(f"unknown --only {only}")
    global palettes_only
    palettes_only = only == "palettes"
    if palettes_only:
        only = None
    disc = Disc.open(version)
    g = Globals(disc)
    out = os.path.join(ROOT, "build", *([] if version == "en" else [version]), "content", "images")
    sectors = version_values(version)             # sector numbers, from include/version.h
    if only in (None, "rooms"):
        extract_rooms(disc, g, out, sectors, room)
    if only in (None, "map"):
        extract_map(disc, g, out)
    if only in (None, "startup"):
        extract_startup(disc, out, sectors)
    if only in (None, "lanim"):
        extract_lanim(disc, g, out, sectors)
    print(f"wrote {os.path.relpath(out, ROOT)}")


if __name__ == "__main__":
    main()
