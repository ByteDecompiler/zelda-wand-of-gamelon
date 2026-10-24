# Compiler

Zelda: The Wand of Gamelon was written in C and compiled to 68000 machine code. The code was compiled on a Macintosh with Apple MPW C 3.0, and then converted into an OS-9 program for the CD-i.

## How the compiler was found

This was first worked out for Link: The Faces of Evil, which was built the same way. You can tell that `cdi_zelda` was compiled on a Mac from:

- Globals are reached through register A5 at negative offsets ((the Mac "A5 world")).
- Functions use `link a6` frames, `int` is 32 bits, and every function has its name stored as a MacsBug symbol after its `rts`.

So it's Apple MPW C. To find the exact version, small test functions were compiled with every MPW C we could find and compared with the game:

- C 3.1 ((1989)) and C 3.2 to 3.3.1 ((1991 to 1993)) each match most functions, but each one gets some things wrong. C 3.1 compares chars as longs, and every C from 3.2 on removes arguments from the stack with `addq.w`, where the game always uses `addq.l`.
- C 3.0 ((Final, 12/20/88)), from the MPW 3.0/3.1 CD, has all of the game's habits: 16-bit array index maths, `cmp.b` for char compares, `addq.l` for stack cleanup, and the same choice of registers. It matched with no changes needed, and it's what this project compiles with too.

## The emulated Mac

MPW C 3.0 is a classic Mac OS program so the build runs it in Basilisk II, a 68k Mac emulator. This repository only has the scripts. You need:

- Basilisk II, built from source ((kanjitalk755/macemu)) in WSL with SDL2 and the JIT ((`--with-gtk=no`)). `emu/mpwrun.sh` expects it at `~/foe_tools/macemu/BasiliskII/src/Unix/BasiliskII`.
- A 68040 Mac ROM that you own, at `emu/Quadra630.ROM` ((a Quadra 630 ROM was used)).
- `emu/System753.dsk`: a System 7.5.3 disk with MPW 3.3 installed ((`tools/scripts/install_mpw.py`)), and MPW C 3.0 installed in `MPW:Tools:` as the tool `C30`. Keep a clean copy of the disk in `emu/pristine/`.
- The two startup files from `tools/mac/` on the Mac: `Run MPW` in the Startup Items folder, and `UserStartup•FOE` in the MPW folder.

## Compiling without clicking

`emu/mpwrun.sh <job folder>` ((run in WSL)) runs one compile job:

1. The job folder holds `build.mpw` ((a list of MPW commands)) and the source files. Text files are converted to the Mac's character set and line endings, and copied to `emu/share/`, which the Mac sees as a disk called "Unix".
2. Basilisk II starts. Its settings are `emu/basilisk_prefs.template` plus the paths on your machine. When the Mac starts up, `Run MPW` opens MPW, and `UserStartup•FOE` copies the job to the Mac's own disk, runs `build.mpw`, copies everything back, and shuts the Mac down, which closes Basilisk II.
3. The results ((`log.txt`, `status.txt` and the object files)) come back into the job folder.

A run takes about 12 seconds. `tools/build.py` compiles all the files in one run.

Some things that took a while to figure out:

- MPW C can't open files on the shared disk ((error -120)), and `Duplicate` fails there ((-43)). Thats why the job is copied to the Mac's own disk with `Catenate`.
- MPW only runs scripts whose file type is TEXT, so the script sets that for every text file.
- If a run has to be stopped by force, the Mac's disk is left "not shut down properly", and the next start-up stops at a dialog. `mpwrun.sh` then restores the clean copy from `emu/pristine/`.
