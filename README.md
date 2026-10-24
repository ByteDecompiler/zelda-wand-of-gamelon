# Zelda: The Wand of Gamelon Decompilation

This is a **decompilation** of ***Zelda: The Wand of Gamelon*** ((Philips CD-i, 1993)). The purpose of the project is to recreate a source code base for the game's main program, `cdi_zelda`, using information found inside the game. **It is not producing a PC port.** The C code compiles back into the exact original program, using the same compiler the game was built with: Apple MPW C 3.0.

```diff
- WARNING! -

This repository is a work in progress. It builds a matching program, but the naming and
documentation are not complete, so the code may still change a lot at any time.
```

The game runs on the same engine as *Link: The Faces of Evil*, so this project started from the [Link: The Faces of Evil decompilation](https://github.com/ByteDecompiler/link-faces-of-evil). All 359 functions are decompiled to C ((96.5% of the code bytes)). 280 of them matched with Link's code unchanged and the rest needed other numbers or real changes. The only parts still in assembly are the MPW runtime and C startup, a few drawing routines, and the two NVRAM name coders. Functions use the game's original names ((from its MacsBug symbols)), and globals, structs and library calls are named too.

It builds the following programs:

* `build/cdi_zelda.mod` ((English)) `sha1: d54c21c5ebed49940f7eb2728b6b1c106d282ffd`
* `build/fr/cdi_zelda.mod` ((French)) `sha1: 93e2a9ea43c64f41b4c9c993e228b2e648662aa6`
* `build/de/cdi_zelda.mod` ((German)) `sha1: 775c43509b2421a40d769bd2e78049c5a45f02a4`
* `build/nl/cdi_zelda.mod` ((Dutch)) `sha1: c0c8f42413869330febb1359dd64511d0de1b92d`

**Note: This repository does not include any of the game's code or data. A prior copy of the game is required to extract them.**

## Supported discs

* English: `Zelda - The Wand of Gamelon (Europe).bin` `sha1: 79190b82602650dffc3b05a5d6f0c546a347c7e6`
* English: `Zelda - The Wand of Gamelon (USA).bin` `sha1: 1d4842e521a8b8e9058283f8440be3013d4401e2`
* French: `Zelda - The Wand of Gamelon (France).bin` `sha1: cbe814af4317f911ddb50312d1af22b45f00ea14`
* German: `Zelda - Der Zauberstab von Gamelon (Germany).bin` `sha1: d9d4c11039fb62517a05cfe9d7928920b2ee141b`
* Dutch: `Zelda - De Toverstaf van Gamelon (Netherlands).bin` `sha1: adc107c2b9207dd8e16162f2573f53ff076dac4d`

The Europe and USA discs have the same program so either one builds the English version.

Everything under `disc/`, `extract/`, `asm/`, `bin/`, `build/` and `ghidra_project/` comes from your disc. Its ignored by git and should never be committed.

## Installation

### Windows

For Windows 10 or 11, install WSL and a distribution by following this [WSL Installation Guide](https://learn.microsoft.com/en-us/windows/wsl/install). We recommend using Ubuntu as the Linux distribution. Then follow the Linux instructions below.

### Linux ((Native or under WSL))

#### 1. Install build dependencies

The build process has the following requirements:

* Python 3
* A Python virtual environment with Capstone, for the disassembly tools:

```bash
python3 -m venv ~/foe_tools/venv
~/foe_tools/venv/bin/pip install capstone
```

The compiler runs in an emulated Mac, so you also need:

* Basilisk II, built from kanjitalk755/macemu with SDL2
* A 68040 Mac ROM that you own, at `emu/Quadra630.ROM` ((a Quadra 630 ROM was used))
* A System 7.5.3 disk image at `emu/System753.dsk`, with MPW 3.3 ((`tools/scripts/install_mpw.py`)) and MPW C 3.0 ((12/20/88, from the MPW 3.0/3.1 CD)) installed as the tool `C30`

Apple's software is not included in this repository. `emu/mpwrun.sh` runs a compile job in the emulator without any clicking. See [docs/emulator.md](docs/emulator.md) for more information.

#### 2. Clone the repository

Clone the repository where you wish to have the project, with the command:

```bash
git clone https://github.com/ByteDecompiler/zelda-wand-of-gamelon.git
```

This will copy the GitHub repository contents into a new folder in the current directory called `zelda-wand-of-gamelon`. Change into this directory before doing anything else:

```bash
cd zelda-wand-of-gamelon
```

#### 3. Prepare a base disc

Place a copy of the game disc ((`.bin` and `.cue`)) inside the `disc/EN/` folder, for example `disc/EN/Zelda - The Wand of Gamelon (Europe).bin`. Check its SHA-1 against the list above.

#### 4. Extract the program

Extract the program from the disc and split it into pieces with the following commands:

```bash
python3 tools/scripts/cdi_modules.py "disc/EN/Zelda - The Wand of Gamelon (Europe).bin" extract/modules
~/foe_tools/venv/bin/python -I tools/split.py
```

This extracts the program's code and data to `asm/` and `bin/`, and writes `config/functions.txt` and `config/jumptable.txt`.

#### 5. Build the program

Run the build script:

```bash
python3 tools/build.py
```

This compiles every function in `src/` in the emulator, links the program and checks its hash.

If all goes well, a new program should be built at `build/cdi_zelda.mod`, and the following text printed:

```
359 of 359 functions from C (96.5% of the code bytes)
build/cdi_zelda.mod: sha1 d54c21c5ebed49940f7eb2728b6b1c106d282ffd
OK: byte-identical to the original
```

If you instead see `NOT byte-identical`, the built program isn't the same as the original one so something went wrong or some part of the code doesn't match. Any function that doesn't match is listed as `DIFFERS`.

**Note:** `--no-compile` skips the compile and reuses the last objects ((handy if you only changed config files)). `--diff` shows each function that doesn't match next to the game's code.

## Changing build options

By default, the English version is built. To build another version, put its disc in `disc/FR/`, `disc/DE/` or `disc/NL/`, and run the same steps with `--version fr`, `de` or `nl`. For example, for German:

```bash
python3 tools/scripts/cdi_modules.py "disc/DE/Zelda - Der Zauberstab von Gamelon (Germany).bin" extract/DE/modules
~/foe_tools/venv/bin/python -I tools/split.py --version de
python3 tools/build.py --version de
```

All versions are built from the same C code. The differences are in `include/version.h`, selected at build time with `-d FRENCH`, `-d GERMAN` or `-d DUTCH`: disc positions, buffer sizes, and the layout of text whose length depends on the language. The English version also has two functions the others don't ((`play_animations` and `play_all_animations`)).

The globals are at different offsets in every version because the texts have different lengths. These are listed in `config/<version>/symbols.txt`, made by `tools/port_symbols.py`.

`--mod` builds a modified program. See [docs/modding.md](docs/modding.md).

## Project Structure

```
zelda-wand-of-gamelon
├── asm: assembly pieces, extracted from your program (generated)
├── bin: module header and data, extracted from your program (generated)
├── build: output directory
├── config: what the build needs to know about the program
│   ├── functions.txt: every code piece in module order
│   ├── symbols.txt: globals and library routines, with their offsets
│   ├── jumptable.txt, segments.txt, relocs.txt: MPW jump table, code segments, references to fix in mods
│   └── fr, de, nl: files for the other versions
├── disc: your disc images (not included)
├── docs: documentation
├── emu: the emulated Mac that runs the compiler
├── include
│   ├── game.h: globals (with their offsets), structs, prototypes
│   └── version.h: values that differ between versions
├── src: C source code, one file per function
└── tools: build tools
    ├── build.py, split.py: the build and the splitter
    ├── port_symbols.py: finds the globals' offsets in another version
    ├── patch_disc.py: writes a built program into a copy of the disc image
    ├── cdi_fs.py, extract_*.py, a5init.py: disc reader and content extractors
    ├── ghidra_setup.ps1, ghidra_scripts: a Ghidra project with the program named and analysed
    └── scripts: comparison, disassembly and setup helpers
```

## Documentation

* [docs/data.md](docs/data.md): the files on the disc, the game's tables, the display, loading and saving.
* [docs/mechanics.md](docs/mechanics.md): the game's mechanics ((items, damage, the shop, rooms)), as written in the code.
* [docs/bugs.md](docs/bugs.md): bugs in the original code.
* [docs/modding.md](docs/modding.md): building mods, testing them in an emulator, extracting graphics and sound.
* [docs/emulator.md](docs/emulator.md): how the compiler was found, and the emulated Mac that runs it.

### How it works

`cdi_zelda.mod` is an OS-9 module. Its code was built on a Mac with MPW C 3.0 ((December 1988)), then converted to OS-9. Globals are reached through register A5, and every function has its name stored as a MacsBug symbol.

`tools/split.py` splits the module into 364 pieces of code: 346 named functions, 13 C functions without a name ((the NVRAM save routines)), 2 assembly routines called like C functions, and 3 stretches of assembly between functions.

`tools/build.py` compiles each function and puts it in its place in the module, using `config/symbols.txt` to find globals and routines by name. It then recomputes the module's CRC and checks its SHA-1.

To use Ghidra ((in PowerShell)), run `pwsh tools\ghidra_setup.ps1`, then open `ghidra_project\WandOfGamelon.gpr`.

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

`main` has to build a matching program for every version. See [CONTRIBUTING.md](CONTRIBUTING.md) to get started.
