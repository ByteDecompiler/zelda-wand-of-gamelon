"""Decode the game's sounds from the disc image to WAV files in build/content/audio/
(build/<version>/content/audio/ for the other versions).

Usage: python3 -I tools/extract_audio.py [--version fr|de|nl] [--only music|voice|sfx|anim|bumper]
The output is game data: it stays in build/, which is never published.

What it writes (sector numbers are the file's own, as the program lseeks to them):
  music/track_NN.wav   zmusic.rtr: 2-minute blocks of 75 * 120 sectors, 8 tracks interleaved in each; track
                       t is channel t % 8 of block t / 8, up to that channel's end-of-record sector
                       (start_music): 10 blocks, 76 tracks, one per room (a room plays the track with its
                       own number; block 9 has channels 0-3 only). After the blocks, at sector $15F90, the
                       world map theme: one stereo 37.8 kHz stream on channel 0 (play_overworld_music; main
                       marks it as track $4C) -> music/overworld.wav.
  voice/voice_NN.wav   zdata.rtr: clip n < 16 is channel n of the record at VOICE_SECTOR, clip n >= 16 is
                       channel n - 16 of the one at VOICE_SECTOR_2 (start_voice_over); each clip ends at its
                       channel's end-of-record sector. The game plays clips 0-$18 (voice_buf, the help)
                       and $1A (clean_disc); all channels found are written (27 clips).
  sfx/sfx_NN.wav       zdata.rtr at EFFECTS_SECTOR: one record with the six effect channels (load_effects).
                       Channel c is loaded into its own buffer, sound_len[c] blocks of $900 bytes (a sector's
                       sound data), and play_sound plays effect id i as the whole of channel sound_chan[i]
                       (ac_process steps through it with chan_offsets), so there are six distinct sounds;
                       sfx_NN is channel NN. sound_chan and sound_len are read from the starting values of
                       the globals in cdi_zelda on the disc.
  anim/anim_NN.wav     zanim.rtr: record n is the 1500 sectors from (n - 1) * 1500 (play_animation); its
                       audio is channel 0 (audio mask 1). Records without audio are skipped (1, 57-59: the
                       background records). Records 61-69 hold 37.8 kHz stereo audio.
  bumper/bumper.wav    bumper.rtf, the logo intro: its audio is on channel 15.
  (The music tracks are stereo sectors with only the left side used, the sound effects with only the
  right; attenuation() routes each side to both speakers, so such streams are written as mono files.)

Sound format (CD-i ADPCM, the same as CD-ROM XA): an audio sector is Form 2 with the audio submode bit; its
coding byte says bit 0 stereo, bit 2 18.9 kHz (else 37.8 kHz), bit 4 8 bits per sample (else 4). The first
$900 bytes of the payload are 18 sound groups of 128 bytes (the last 20 bytes of the payload are unused).
A group starts with 16 header bytes, the sound parameters of its sound units, each repeated (for 4 bits:
the eight units' parameters are at bytes 4-11, with 0-3 a copy of 4-7 and 12-15 a copy of 8-11; for 8 bits
the four units' parameters are at bytes 0-3, repeated four times), then 112 data bytes: 28 rows of 4
bytes. In 4-bit mode a row holds sample j of all eight units (unit u in byte u / 2, low nibble for even
units); in 8-bit mode one byte per unit (four units). A parameter byte holds the range (shift) in its low
nibble and the filter in bits 4-5. Each sample is (signed value << 12 (or 8)) >> range, plus the filter's
prediction from the previous two outputs: K0 * s[-1] + K1 * s[-2], with (K0, K1) = (0, 0), (0.9375, 0),
(1.796875, -0.8125), (1.53125, -0.859375), i.e. (0, 60, 115, 98) and (0, 0, -52, -55) / 64. Mono streams
use the units in order; stereo streams alternate left (even units) and right (odd units). The filter state
carries on from group to group and sector to sector, separately for each side.
"""
import argparse
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import build      # noqa: E402  (its --version option picks the version's symbols)
import a5init     # noqa: E402
from cdi_fs import VERSIONS, Disc, version_values  # noqa: E402
from media import write_wav     # noqa: E402

OUT = os.path.join(ROOT, "build", "content", "audio")

MUSIC_BLOCK = 75 * 120          # sectors in a 2-minute music block (start_music)
OVERWORLD_SECTOR = 0x15F90      # play_overworld_music (lseek to $AFC8000, the same in every version)
ANIM_RECORD = 75 * 20           # sectors in a zanim.rtr record (play_animation)
BUMPER_CHANNEL = 15


def effect_tables(disc):
    """sound_chan[16] (effect id -> channel) and sound_len[6] (blocks of $900 bytes per channel), from the
    starting values of the globals in the program module on the disc (unpacked as tools/a5init.py does)."""
    size = disc.files["cdi_zelda"][1]
    mod = b"".join(s.payload for _, s in disc.sectors("cdi_zelda"))[:size]
    img, _ = a5init.unpack(mod)
    syms = {n: v for n, (kind, v) in build.load_symbols().items() if kind == "data"}

    def get(name, n):
        p = a5init.BELOW + syms[name]                  # data symbols are negative A5 offsets
        return list(img[p:p + n])
    return get("sound_chan", 16), get("sound_len", 6)

K0 = (0, 60, 115, 98)
K1 = (0, 0, -52, -55)


def _tables():
    """For each parameter byte, the list of 256 (low nibble, high nibble) shifted sample values for 4-bit
    data (so a data byte gives two units' samples at once), and the 256 shifted values for 8-bit data."""
    t4, t8 = [], []
    for param in range(256):
        shift = param & 0xF
        if shift > 12:                  # not used by valid data; clamp like the hardware's shifter
            shift = 9
        nib = [((n - 16 if n & 8 else n) << 12) >> shift for n in range(16)]
        t4.append([(nib[b & 0xF], nib[b >> 4]) for b in range(256)])
        t8.append([((b - 256 if b & 0x80 else b) << 8) >> shift for b in range(256)])
    return t4, t8


T4, T8 = _tables()


def decode(payloads, coding):
    """Decode a channel's audio sector payloads (all with the same coding) to 16-bit samples, interleaved
    left/right when stereo. Returns (samples, rate, channels)."""
    stereo = coding & 1
    rate = 18900 if coding & 4 else 37800
    eight = coding & 0x10
    units = 4 if eight else 8
    # filter state: [s1, s2] per side
    state = [[0, 0], [0, 0]]
    out = []
    for p in payloads:
        for g in range(0, 18 * 128, 128):
            grp = p[g:g + 128]
            rows = [grp[16 + 4 * j:20 + 4 * j] for j in range(28)]
            unit_samples = []
            for u in range(units):
                if eight:
                    param = grp[u]
                    tab = T8[param]
                    vals = [tab[r[u]] for r in rows]
                else:
                    param = grp[4 + u]
                    tab = T4[param]
                    half = u & 1
                    byte = u >> 1
                    vals = [tab[r[byte]][half] for r in rows]
                side = state[u & 1] if stereo else state[0]
                k0, k1 = K0[(param >> 4) & 3], K1[(param >> 4) & 3]
                s1, s2 = side
                res = []
                for v in vals:
                    s = v + ((s1 * k0 + s2 * k1 + 32) >> 6)
                    if s > 32767:
                        s = 32767
                    elif s < -32768:
                        s = -32768
                    res.append(s)
                    s2, s1 = s1, s
                side[0], side[1] = s1, s2
                unit_samples.append(res)
            if stereo:
                for u in range(0, units, 2):
                    left, right = unit_samples[u], unit_samples[u + 1]
                    for j in range(28):
                        out.append(left[j])
                        out.append(right[j])
            else:
                for res in unit_samples:
                    out.extend(res)
    return out, rate, 2 if stereo else 1


def read_channels(disc, name, start, channels, stop_at_any_eor=False, count=None):
    """Read the audio sectors of the given channels from sector `start` on. Each channel's sound ends at its
    own end-of-record sector (a PCB that selects only that channel), or, with stop_at_any_eor, all end at
    the first end-of-record sector of a selected channel (one PCB selecting them all). Returns
    {channel: (coding, [payloads])} for the channels that had audio."""
    found = {}
    done = set()
    for _, s in disc.sectors(name, start, count):
        ch = s.channel
        if ch not in channels or ch in done:
            continue
        if s.audio and s.form2:
            coding, payloads = found.setdefault(ch, (s.coding, []))
            if s.coding != coding:
                print(f"  {name} channel {ch}: coding changes from {coding:#x} to {s.coding:#x}", file=sys.stderr)
            payloads.append(s.payload)
        if s.eor:
            if stop_at_any_eor:
                break
            done.add(ch)
            if len(done) == len(channels):
                break
    return found


def save(path, coding, payloads):
    samples, rate, nch = decode(payloads, coding)
    note = ""
    if nch == 2:
        # the music uses only the left side of its stereo sectors, the sound effects only the right;
        # attenuation() sends both sides to both speakers, so such a stream is written as mono
        for used, name in ((0, "left"), (1, "right")):
            if not any(samples[1 - used::2]):
                samples, nch, note = samples[used::2], 1, f"  ({name} side only)"
                break
    os.makedirs(os.path.dirname(path), exist_ok=True)
    write_wav(path, samples, rate, nch)
    seconds = len(samples) / nch / rate
    peak = max((abs(x) for x in samples), default=0)
    print(f"  {os.path.relpath(path, os.path.dirname(os.path.dirname(path))):<22} {len(payloads):5} sectors  {seconds:7.2f} s  "
          f"{rate} Hz {'stereo' if nch == 2 else 'mono  '}  peak {peak}{note}")


def extract_music(disc, out):
    blocks = -(-OVERWORLD_SECTOR // MUSIC_BLOCK)
    for b in range(blocks):
        found = read_channels(disc, "zmusic.rtr", b * MUSIC_BLOCK, set(range(8)), count=MUSIC_BLOCK)
        for ch in sorted(found):
            save(os.path.join(out, "music", f"track_{b * 8 + ch:02}.wav"), *found[ch])
    found = read_channels(disc, "zmusic.rtr", OVERWORLD_SECTOR, {0})
    if 0 in found:
        save(os.path.join(out, "music", "overworld.wav"), *found[0])


def extract_voice(disc, out, sectors):
    for first, base in ((sectors["VOICE_SECTOR"], 0), (sectors["VOICE_SECTOR_2"], 16)):
        found = read_channels(disc, "zdata.rtr", first, set(range(16)))
        for ch in sorted(found):
            save(os.path.join(out, "voice", f"voice_{base + ch:02}.wav"), *found[ch])


def extract_sfx(disc, out, sectors):
    SOUND_CHAN, SOUND_LEN = effect_tables(disc)
    found = read_channels(disc, "zdata.rtr", sectors["EFFECTS_SECTOR"], set(range(6)), stop_at_any_eor=True)
    for ch in sorted(found):
        coding, payloads = found[ch]
        if len(payloads) != SOUND_LEN[ch]:
            print(f"  sfx channel {ch}: {len(payloads)} sectors, sound_len says {SOUND_LEN[ch]}", file=sys.stderr)
        save(os.path.join(out, "sfx", f"sfx_{ch:02}.wav"), coding, payloads[:SOUND_LEN[ch]])
    for ch in range(6):
        ids = [i for i, c in enumerate(SOUND_CHAN) if c == ch]
        print(f"  sfx_{ch:02}: effect ids {', '.join(map(str, ids))}")


def extract_anim(disc, out):
    first, size = disc.files["zanim.rtr"]
    records = -(-(-(-size // 2048)) // ANIM_RECORD)
    for n in range(1, records + 1):
        # take all of the 1500 sectors: some records hold several end-of-record sectors (background
        # animations played record by record), with audio after them
        payloads, coding = [], None
        for _, s in disc.sectors("zanim.rtr", (n - 1) * ANIM_RECORD, ANIM_RECORD):
            if s.channel == 0 and s.audio and s.form2:
                coding = s.coding if coding is None else coding
                payloads.append(s.payload)
        if payloads:
            save(os.path.join(out, "anim", f"anim_{n:02}.wav"), coding, payloads)


def extract_bumper(disc, out):
    payloads, coding = [], None
    for _, s in disc.sectors("bumper.rtf"):
        if s.channel == BUMPER_CHANNEL and s.audio and s.form2:
            coding = s.coding if coding is None else coding
            payloads.append(s.payload)
    if payloads:
        save(os.path.join(out, "bumper", "bumper.wav"), coding, payloads)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--version", choices=VERSIONS, default="en")
    ap.add_argument("--only", choices=("music", "voice", "sfx", "anim", "bumper"))
    args = ap.parse_args()
    disc = Disc.open(args.version)
    out = OUT if args.version == "en" else os.path.join(ROOT, "build", args.version, "content", "audio")
    sectors = version_values(args.version)        # zdata.rtr sector numbers, from include/version.h
    print(os.path.basename(disc.path), "->", out)
    kinds = [args.only] if args.only else ["sfx", "voice", "bumper", "anim", "music"]
    for kind in kinds:
        print(kind)
        if kind == "music":
            extract_music(disc, out)
        elif kind == "voice":
            extract_voice(disc, out, sectors)
        elif kind == "sfx":
            extract_sfx(disc, out, sectors)
        elif kind == "anim":
            extract_anim(disc, out)
        else:
            extract_bumper(disc, out)


if __name__ == "__main__":
    main()
