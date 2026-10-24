# Contributing

How to work on the decomp. Setup is in the [readme](README.md).

## Getting started

You'll want to know some C, mostly arrays and pointers. 68000 assembly helps but you can learn it along the way.

You'll need git and a GitHub account.

Get the [build](README.md#installation) working first and check that it matches before you change anything.

## What's left

Matching is done: all 359 functions are C that compiles to the exact same bytes as the game. The decomp started from [Link: The Faces of Evil](https://github.com/ByteDecompiler/link-faces-of-evil), which runs on the same engine, and 280 functions matched with Link's code unchanged. Things that could still use work:

- **Naming and documenting.** A lot of names still come from Link and some won't fit this game ((like the sword, glove and water globals, which are Link's names for Zelda's items)). If you're not sure what something does its fine to leave it unnamed for now.
- **The last bits of assembly.** A few drawing routines and the two NVRAM name coders are still assembly ((the MPW runtime and C startup are library code, so they stay as they are)).
- **Initialisers.** The build can't handle globals with starting values yet, so the data tables aren't in the C.
- **Docs.** The mechanics and bugs pages can always grow, and some of the bugs marked "maybe" could be checked.

## Redoing a function

This is how each function was decompiled. Use the same steps to redo one for example to give it better names or types, or to turn one of the assembly pieces into C. Link's version of a function is often a good place to start.

1. Pick a function. `~/foe_tools/venv/bin/python -I tools/scripts/gamefuncs.py extract/modules/cdi_zelda.mod --list` lists them from smallest to biggest. Read it in Ghidra or in `asm/<name>.s`.

2. Make `src/<name>.c`, starting with `#include "game.h"`.

3. Every global it uses needs a declaration in `include/game.h` and a line in `config/symbols.txt`: `data <name> -0x<offset>`.

4. `jsr N(a5)` is a call into another code segment. Look up N in `config/jumptable.txt` to see what it calls. Game functions are linked by name. Library routines at the end of the module need a `jt <name> 0x<N>` line, and routines inside the assembly pieces need a `code <name> 0x<address>` line.

5. Run `python3 tools/build.py`. A function that compiles but doesn't match shows up as `DIFFERS`. Add `--diff` to see what's different, or run `~/foe_tools/venv/bin/python -I tools/scripts/compare.py extract/modules/cdi_zelda.mod build/obj/<name>.o -d`.

6. If you added globals, run `python3 tools/port_symbols.py --version fr` ((and again with `de` and `nl`)) to find their offsets in the other versions.

## Style

- Functions keep the game's own names ((its MacsBug symbols)), typos and all, like `check_vulnerablility`. The NVRAM routines have no names in the game, so they're called `nvr_*`.
- Globals, struct fields and library calls are named in `snake_case`, after what the code does with them. Every `extern` in `include/game.h` has a comment with its A5 offset and what it holds.
- Each source file starts with a comment saying what the function does. If the code only matches when written in a weird way, write it that way anyway and add a comment.
- Numbers that change between versions live in `include/version.h`. When the code itself changes, that file has a flag for it ((e.g. `NEW_SAVE_CHECK`)), and the function checks the flag, never the language.

## Pull requests

Before opening a PR, check that:

- `python3 tools/build.py` still matches.
- `--version fr`, `--version de` and `--version nl` still match too.
- If you added globals, you ran `tools/port_symbols.py` for `fr`, `de` and `nl`.
- You didn't add any game code or data ((nothing from `disc/`, `extract/`, `asm/`, `bin/`, `build/` or `ghidra_project/`)).
- Names and comments are spelled right.

Keep mods off `main` and on a seperate branch (([docs/modding.md](docs/modding.md) has more on mods)).

## Matching tips ((MPW C 3.0))

Stuff we ran into while matching Link: The Faces of Evil, which used the same compiler. Function names in brackets are where it came up, and most of them are in this game too.

### Types and variables

The size suffix gives you the type:

```
.b      char
.w      short
.l      long
moveq #0,d0; move.w ...     unsigned short
```

Parameters get 4 bytes each. Chars and shorts sit at the end of their slot:

```
$b(a6)          first parameter, as a char
$a(a6)          first parameter, as a short
move.w $e(a6)   (short)f, the low word of a long parameter
```

- The first local declared gets the lowest stack address.
- Register variables: data goes in d7, then d6 and down. Pointers go in a3, then a4 and up.
- Locals declared `register` get the top registers first. The rest are ranked by how often they're used. To get a variable into d7, declare it `register`.
- `register` variables are still ranked by use among themselves. To put a less used variable above a more used one, declare only that one `register` ((`j` and `k` in init_enemies)).
- Two variables that are never alive at the same time can share a register. The most used variables get registers first, and on a tie the later declaration wins.
- `register` doesn't always help. It moves `n` to the top in check_screen_edges, but does nothing in lob_bomb.
- One register holding two unrelated values ((say a swap temp, then a coordinate)) means two variables in the C ((`t` in check_box, `dy` and `speed` in lob_bomb)).
- Don't name anything `entry`. It's a reserved word in MPW C 3.0 ((an old K&R keyword)).

### Arrays and pointers

- If a function keeps a global array's address in a3 or a4 and indexes it in 16 bits ((`lea g(a5),a3 ... (a3,d0.w)`)), the array is defined in that function's own file ((like `long heart_x[11];`, no initialiser)). `extern` arrays get reloaded every time they're used ((`lea g(a5),a0`)). So the arrays a function keeps in registers tell you which file it was in. With two such arrays, the first one defined goes in a4. The build matches defined globals to their offsets in `config/symbols.txt` ((initialisers aren't supported yet)).
- That's why `game.h` only has `extern` declarations. An array defined there would be defined in every file, and every function using it could keep it in a register. `proj_draw`, for example, is only defined in the files of draw_projectiles, init_enemy_projectiles and load_playfield, and not in drop_sprite's.
- A local array used by name ((`char buf[4]; f(buf);`)) gets its address put in an address register ((`lea -4(a6),a3`)). Using a pointer variable instead adds a `movea`.

```c
extern struct smap *smaps[100000];  /* asl.l #2; move.l (a0,d0.l): 32-bit index (restore_smaps) */
extern struct smap *smaps[];        /* 16-bit index, same with any size under 32 KB */

proj_defs[k + 1].a                  /* movea.l $c(a0,d0.l) after k * 12: the + 1 is folded in */

*(char *)(p + 0x50)                 /* cmp.b $50(a0) */
((char *)(p + 0x50))[0]             /* moveq #$50,d1; add.l d1,d0; movea.l d0,a0; cmp.b (a0) (check_if_on_screen) */

++e                                 /* adda.w #size */
e = (struct enemy *)((char *)e + 0xb0L);   /* adda.l */

q = &p[1];                          /* movea.l p,a0; addq.l #1,a0; move.l a0,q */
q = p + 1;                          /* uses the data register that still holds p (check_enemy_combat) */
```

### Operand order

Order matters. Each pair here compiles differently:

```c
0x40000000 | (x & m)        (x & m) | 0x40000000
x + y * 704                 y * 704 + x             /* the game does y * 704 first, so write it first */
```

- `a + b` with two globals, or a global and a register variable, loads `b` and adds `a`. The game's `move.l plane_a_mem,d0; add.l y,d0` is `y + plane_a_mem`. Swap them if yours comes out backwards.
- With a global and a struct field, the order decides which ends up in d1 ((check_map)):

```c
pf_y + projectiles[p].map_y     /* global added to the field in d0 */
projectiles[p].map_y + pf_y     /* global loaded into d1, then d0 added to it */
```

- When comparing with a register variable, put the variable on the left ((`y >= tools[i].y - 2`)). Then the computed value is in d0 and you get `cmp.l d5,d0`. Variable on the right gives `cmp.l d0,d5`.
- Sum in the wrong order? Try grouping it another way. `font_height + 25 + font_height + 2 + ...` matched, `font_height + (font_height + 25)` didn't.

### Arithmetic

```c
x += 1;  ++i;           /* use these, not x++; (which loads the old value first) */
t = t + 32;             /* moveq; add.l */
t += 32;                /* addi.l */
x + -6                  /* moveq #$fa,d3; add.l d3,d0 */
x - 6                   /* subq.l #6 */
x % 65536               /* andi.l #$ffff, when x is unsigned */
x & 0xffff              /* loads the mask into a register when the result is stored */
random_seed % 2         /* andi.l #1, when unsigned (any power of two works the same) */
x & 1                   /* loads the 1 into a register first */
(long)c % 18            /* calls LMODT, c is an unsigned char */
c % 18                  /* divs.w */
```

The MPW runtime routines are all in gap_04226 ((segment 1)):

```
$44D4   ULMULT   long *
$44F8   ULMODT   unsigned %
$4506   ULDIVT   unsigned /
$4516   LMODT    long %
$453C   LDIVT    long /
```

In C 3.0 a sum of unsigned chars stays unsigned, so dividing one calls ULDIVT.

### Conditions and loops

```c
while (f(...), flags &= 3);             /* call in a loop test */
for (;;) { ...; if (!x) break; ... }    /* loop that tests in the middle */
```

- `for` loops:
  - If the start value already passes the test, there's no jump to the test at the top, but d3 still gets saved ((mmenu_scroll_up)).
  - A plain constant limit is kept in d3 ((`move.l #$de,d3; cmp.l d7,d3`)).
  - `cmpi.l #$de,d7`, or `tst.l` against 0, means it was a `while` loop ((scroll_position)).
- Argument pops:
  - A call inside a condition, like `if ((p = f(x)) == -1)`, has its arguments popped right after the test, even if only a return follows.
  - A call in its own statement before the `if` isn't popped there.
- `?:` results:
  - `c ? 1 : 2` tested with `bne` to the 2 is really `c == 0 ? 1 : 2`.
  - `!c ? 1 : 2` gives `seq` and `neg`.
- `x + 0 > 59` ((probably a macro adding an offset of 0)) loads x into a register before comparing. The game does this in check_screen_edges and check_if_on_screen. Plain `x > 59` compares straight from memory.
- A char register variable sign-extended in place ((`ext.w d7; ext.l d7; cmp.l d7,d0`)):

```c
--cur;
if (cur == -1)          /* matches */
if (--cur == -1)        /* copies it into d0 first */
```

- `move.l d0,d4; subq.l #1,d4; tst.l d0` ((copy, decrement, test the old value)) is `w = m[0x39]; if (!w--) return;` ((check_tool_collision)).
- A condition passed as an argument: `a < b` gives `slt; andi.w #1; ext.l`. The game's `moveq #0,d2; ...; sge.b d2; neg.b d2` is `!(a < b)` ((check_sprite_collisions)).
- Returning 1 or 0 ((move_types_1_and_3)):

```c
if (a && f()) return 1; return 0;           /* same code as return a && f(); moveq #0,d0 first, kept on the stack */
if (a) { if (f()) return 1; } return 0;     /* plain moveq #1,d0 / moveq #0,d0, like the game */
```

- No stack pop after the last call means that call is the last thing the function does at its top level. Put `if (x) return;` before it instead of wrapping it in `if (!x) { ... }`.
- `?:` vs `if`/`else` ((big_jump_down)): look for `moveq #0,d0; move.l d0,g` ... `moveq #1,d1` for an argument, inside one branch. The compiler still remembers the 0 in d0, so it's a `?:`, like `zelda_dir == 2 ? f(1) : f(-1);`. An `if`/`else` makes it forget d0.
- `clr.b` followed by `moveq #0,d0; moveq #0,d0; move.l d0,x` is a chained assignment through a char: `ov_help_repeat = ov_help_cmd = 0;` ((get_help_instruction)).

### Switches

- Case bodies come out in source order ((the jump table is sorted by value)), so write the cases in the order their code appears ((signal_handler)).
- If the jump table starts at 0 but case 0 does nothing, the source has an explicit `case 0: break;` ((mmenu_loop)). Without it, the table starts at the lowest case used.
- `bcs`/`bhi` means a switch on an unsigned char ((`switch ((unsigned char)rope_out)`)). `bmi`/`bgt` means signed.

### Functions and parameters

- A pointer parameter loaded into its address register after the data register parameters is declared `register` in the definition ((init_type_8)). The prototype doesn't need it.
- A char parameter loaded into its register before the long ones ((`move.b $13(a6),d5` first)) is declared `char` in an old-style definition ((`void start_anim(room, n, kind) long room; long n; char kind; {`)). The prototype can still say `long`, since a char is promoted to long.
- A function's address ((like `intercept(signal_handler)` in main)) is its jump table entry ((`pea $10a(a5)`)), even in the same segment. `tools/build.py` links it that way.
- A function with no return type whose caller uses the result ((playfield, void in its own file)) gets a `long` prototype in the caller's file only.

### Tools

- `tools/scripts/compare.py` ignores relocated fields, so `pf_x + zelda_sprite_x` and `zelda_sprite_x + pf_x` look the same to it. Use `tools/build.py` to be sure.
