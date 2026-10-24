# Modding

## Building a mod

1. Change the C code on a new branch ((`main` must stay an exact match)). For example, changing `max_hearts = 3;` to `max_hearts = 6;` in `src/set_life_hearts.c` starts the game with 6 hearts.
2. Run `python3 tools/build.py --mod` ((add `--version fr`, `de` or `nl` for another version)). This builds every function from its C code, whatever its size. If a function grows or shrinks, the code after it moves, and everything that points to moved code is fixed up: calls between C functions, the few references from the assembly pieces and the library (`config/relocs.txt`, made by `tools/relocs.py`), the jump table and the module header. The build lists the functions your mod changed (`MODDED: <name>`).
3. Run `python3 tools/patch_disc.py` ((with the same `--version`)). It copies your disc image to `build/disc/` ((or `build/<version>/disc/`)), writes the new program over the old one, and recomputes the sectors' error correction codes ((EDC/ECC)). The program's file on the disc has room for 135168 bytes, which is 0.7 to 0.9 KB more than the original program depending on the version. With an unchanged build the copy is identical to the original disc.
4. Run the `(mod).cue` file in a CD-i emulator. For example, with MAME:

   ```bash
   mame cdimono1 -bios pcdi220 -cdrom "build/disc/<name> (mod).cue"
   ```

   Then click Play CD-i. MAME needs the CD-i BIOS set ((including the servo and slave chip dumps)) in its `roms/cdimono1.zip`, which you have to supply yourself.

## Extracting content

These tools read your disc image directly ((trough `tools/cdi_fs.py`, which also lists the files on the disc)). They only need Python's standard library. Everything they write goes to `build/content/` ((or `build/<version>/content/` with `--version`)), which is never published.

- `tools/extract_images.py`: the 76 rooms, the world map and its place labels, the item and menu icons, the font, the anti-piracy notice, the animation backgrounds and the palettes ((PNG)).
- `tools/extract_sprites.py`: Zelda's frames, 69 kinds of enemies ((with a list of the rooms they appear in)), items and projectiles ((PNG)). Enemy, item and projectile graphics are small pieces of 68000 code that draw pixels, so the tool runs them in a small 68000 interpreter.
- `tools/extract_audio.py`: 76 music tracks and the world map theme, the voice clips, the sound effects, the cutscene audio and the logo jingle ((WAV, decoded from CD-i ADPCM)).
- `tools/a5init.py`: the starting values of the program's globals ((`build/a5init.txt`)).

The description at the top of each tool explains the formats it reads. [data.md](data.md) describes the files on the disc.
