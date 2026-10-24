"""Small writers for the content extractors (standard library only): PNG images and WAV sounds."""
import struct
import wave
import zlib


def write_png(path, width, height, pixels, palette=None):
    """pixels: bytes, one per pixel (an index into palette, a list of (r, g, b)) when a palette is given,
    else three per pixel (r, g, b). With a palette, index 0 can be made transparent by passing
    palette=(colours, 0)."""
    transparent = None
    if isinstance(palette, tuple):
        palette, transparent = palette
    if palette is not None:
        colour, bpp = 3, 1
    else:
        colour, bpp = 2, 3
    rows = b"".join(b"\0" + bytes(pixels[y * width * bpp:(y + 1) * width * bpp]) for y in range(height))

    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))

    out = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, colour, 0, 0, 0))
    if palette is not None:
        out += chunk(b"PLTE", b"".join(bytes(c[:3]) for c in palette[:256]))
        if transparent is not None:
            out += chunk(b"tRNS", bytes(255 if i != transparent else 0 for i in range(transparent + 1)))
    out += chunk(b"IDAT", zlib.compress(rows, 9)) + chunk(b"IEND", b"")
    open(path, "wb").write(out)


def write_wav(path, samples, rate, channels=1):
    """samples: a list of 16-bit ints, interleaved when channels > 1."""
    with wave.open(path, "wb") as w:
        w.setnchannels(channels)
        w.setsampwidth(2)
        w.setframerate(rate)
        w.writeframes(struct.pack(f"<{len(samples)}h", *(max(-32768, min(32767, s)) for s in samples)))
