# The game's data

The files `cdi_zelda` reads from the disc and the tables in its globals. Only the layouts are written down here, the actual values come from your own disc: `python3 tools/a5init.py` lists the globals' starting values in `build/a5init.txt`, which is never published.

Units used on this page:

- A **cell** is 4 pixels. Positions in the game are mostly in cells ((sprites are drawn at `x * 4`)).
- A **sector** is a 2048-byte CD-i sector. 75 sectors make one second of disc.
- A **screen** is 384 x 240 pixels, or 96 x 60 cells.

## Files on the disc

The program opens three real-time files ((RTR)) and plays records from them with `ss_play`. A CD-i "play" streams sectors into memory buffers, as described by a PCB and PCL lists ((see below)). Each load seeks to a sector with `lseek`, starts `ss_play`, and waits until the signal handler sets `play_done`. If it sets `play_failed` instead, `play_error` closes the data file, opens the next copy of it and tries again. After two retries it gives up, shows the "clean the disc" sign and goes back to the main menu ((or quits if no game is running)).

`zdata.rtr`, opened as `data_path` ((`zdata1.rtr` and `zdata2.rtr` are copies used after read errors, `rtr_index` says which one is open)):

- Sector 0: the world map ((`load_world`)): the palette file, the map image and the label images.
- Sector $AF: the palettes, menu icons, macro data, Zelda's graphics and her collision maps ((`initial_load`)).
- Room `r` at sector `r * $C0 + $C9` ((`load_playfield`)): the enemy list and enemy records, the room's environment projectiles, enemy graphics and collision maps, the palette, the block map and the room image. A full load also brings the projectile graphics and collision maps and the item graphics.
- Sector $394D: voice-over clips 0 to 15. Clip `n` starts at sector $394D + `n` on channel `n % 16` ((`start_voice_over`)). Clips 16 and up start at $3B3A instead.
- Sector $3CE4: the sound effects ((`load_effects`, 6 channels)).
- The other versions use other sector numbers; see `include/version.h`.

`zanim.rtr`, opened as `anim_path`:

- Cutscenes and backgrounds, in records of 20 seconds. Record `n` starts at sector `(n - 1) * 1500`. Used by `play_animation`, `play_background_animation` ((record $33 for the intro of a new game, $37 for the ending)), `load_backgrounds` ((records 57, 58 and 60)) and `load_piracy` ((record $3D, or $41 on a PAL machine)).

`zmusic.rtr`, opened as `music_path` by `init_audio`:

- Music, in 2-minute blocks of 8 tracks played side by side. Track `t` is in block `t / 8`, channel `t % 8` ((`start_music`)). A room plays the track with its own number and the world map plays track $4C.

`/nvr/zelda<n>.nvri`, opened by `nvr_open` and `nvr_create`:

- Saved games, in the player's NVRAM ((see below)).

The intro logo is a seperate program, `cdi_bumper`. `play_bumper` loads that file with `modcload`, links its data module `cdi_bumpdata` ((it comes in the same file, there's no `cdi_bumpdata` on the disc)), hands it the display tables and buffers and runs it with `os9exec`. The disc also has `bumper.rtf`, which `cdi_zelda` never opens itself.

At startup `prepare_mod_dir` loads the two small modules `ersatz_1` and `ersatz_2` and unlinks them again straight away.

### Rooms

There are 76 rooms, numbered 0 to 75.

- `room_widths[76]` and `room_heights[76]`: each room's size in screens. A room scrolls either sideways or up and down, never both ((`init_playfield_size`)).
- `block_map`: one byte per cell of the room, giving the kind of block ((solid, floor, slope, ladder, door, and so on)). `get_sprite_block` turns the blocks around a point into a `struct block_info`.
- Block codes 94, 79, 84, 91, 89 and 77 are the six doors: walking into one leaves the room through exit 0, 1, 2, 3, 4 or 5 ((`drop_sprite`)). Some of them are locked until Zelda has the right key ((`check_for_key`, keys are experience bits $81 to $92)).
- `area_entries[15]` ((10 bytes each, in `main.c`)): where the game enters a room from each place on the world map. Each entry has the room, Zelda's position ((`entry_x`, `entry_y`)) and the scroll position ((`scroll_x`, `scroll_y`)), in cells. [mechanics.md](mechanics.md#places) lists which place is which.
- `area_exits[126]` ((same layout)) and `exit_base[76]`: exit `n` of room `r` is `area_exits[exit_base[r] + n]`. It gives the room on the other side and where Zelda appears there.
- `dark_rooms[]`: 15 rooms that are dark without a lantern ((`set_dark_room`)). They're stored as room number + 1 and a 0 byte follows.
- `tool_spots[55]` ((`struct tool_spot`, 6 bytes each)): items lying in rooms, with their kind, room, x and y. Spots 0 to 17 remember being picked up as experience bits $81 to $92, a lot of spots only show up once another experience bit ($A2 to $B2) is set, and spots 47 to 54 go away when Zelda already has what they give ((`check_if_have_tool`)). Room 18 is the shop, which also puts out bombs, ropes and oil to buy.
- `env_projectiles`: the room's own projectiles, like traps. They come with the room, up to 30 of them, 14 longs each ((the arguments of `init_projectile`)).
- The room graphics start with six colour cycle ranges ((6 first colours, 6 lengths, 6 delays, all shorts)), which `load_playfield` copies to `bg_cycle_start`, `bg_cycle_len` and `bg_cycle_delay`. The world map uses `world_cycle_start`, `world_cycle_len` and `world_cycle_delay` instead.

### Enemies

A room's enemy list ((`enemy_list`)) gives the kinds of enemies in it, ending with -1 ((`enemy_types[]`: an id and a graphics block)). The enemy records follow at +32 ((`enemy_data`, 176 bytes each, `struct enemy`)). A room has up to 20 enemies, sprites 1 to 20 ((sprite 0 is Zelda)).

Each sprite runs a "macro", which is an animation script ((`macros[]`, `sprite_macros`, `macro_ptrs`)). It is moved by one of the `move_type_<n>` routines, chosen by the record's type ((`init_type_<n>`)).

`vulnerability[]` holds one byte per enemy id ((numbered from 1)), with the bits of the weapons that hurt it ((`check_vulnerablility`)): 1 sword beam, 2 bomb, 4 power glove, 8 sword, $10 level-2 sword, $20 and $40 two more weapons. An enemy in combat state 7 can only be hurt by the power glove and bombs.

Enemy graphics are compiled sprites: small pieces of 68000 code that draw the picture. `struct enemy_gfx` gives the code offset of each of the 24 frames ((from +$9C, called with the screen position to draw at)) and a colour for each frame. The sizes in its header give where the next block starts.

Collision maps ((`cmaps[]`)) hold $3A bytes per frame, $570 bytes for a set of 24 frames: 14 rows of hit mask, 14 rows of shield mask, an offset and a width. `cmaps[0]` is Zelda's, 1 to 3 are the room's enemies and the projectiles come after.

### The world map

- `labels[15]` ((`struct label`, 18 bytes each)): the place names shown on the world map. Each has where the label image goes ((x, y, w, h in pixels, the map image is 704 pixels wide)), the hotspot that shows it ((in map coordinates)) and a group number. Labels in the same group belong to one place. Label 15 means no label.
- `label_slots[15]`: where each label's image sits in the loaded label images ((in steps of $914 bytes)).
- `areas[15]` and `initial_area_tab[15]`: which places are open ((1)). `open_up_area` opens one when Zelda gets a map piece ((item spots 18 to 29, through a fixed order of places)). A loaded game opens them again from experience bits $93 to $9E.
- In the English version map spots 15 to 17 don't lead to rooms, they just play animations ((the ending and two animation tests, see `ANIMATION_TESTS` in `include/version.h`)). `anim_sequence[22]` is the list `play_animations` plays.

### Help and demo scripts

- `help_demo`: the help script, a list of bytes. On the world map `get_help_instruction` first walks the cursor to map spot (100, 80), then the script carries on in the rooms with `retrieve_help_state`. A move is a count and then a command byte ((the stick in the low 5 bits: 1 right, 2 left, 4 down and 8 up, and the buttons in the top bits)). $80 to $82 are voice-over commands, $83 gives the level-2 sword and full life, $84 turns Zelda round, $85 shows the "not enough rubies" sign, $86 changes enemy 8, and $FF is the end.
- `voice_buf`: the voice-over clip numbers the help plays, in order ((`voice_ptr`)).
- `mkt_demo_buf`: the attract demo that starts after 1400 idle frames on the title menu. One byte per frame, the stick in bits 0 to 4 and the buttons in bits 5 to 7 ((`retrieve_mkt_demo`)).

## Tables in the globals

- `hero_actions` and `zelda_frames_<n>`: Zelda's actions. Each has an id and a list of frames. A frame list is a list of shorts that ends with 999. `hero_actions` starts out empty, `init_link` fills it in.
- `walk_frames_left`, `walk_frames_right` and `death_frames`: the frame lists of walking enemies and of a dying sprite.
- `jump_heights[4]`: how many cells Zelda rises in each step of a jump.
- `proj_frames_<n>` and `proj_defs`: projectile animations ((frame numbers ending with $FF)) and projectile kinds ((`struct proj_def`, filled in by `setup_projectile_types`)).
- `item_names`: two lines of text per item, for the "you got" sign and the subscreen. It has 28 entries, one per item count.
- `initial_counts[28]`: item counts at the start of a game.
- `mm_text`, `mm_titles` and `mm_rows`: the main menu's options, titles and number of rows, for 7 menus.
- `low_bits_mask`, `high_bits_mask` and `bit_masks`: bit masks for the collision tests. `bit_masks` doubles as the channel masks for music and voice-overs ((`music_chan`)).
- `dcp_regs_a` to `dcp_regs_d` and `color_regs`: display instructions ((FCT)) written at startup and after the logo.
- `cursor_data`: the 16 x 16 pointer image ((`gc_def`)).
- `heart_x[11]` and `heart_w`: where the life hearts are drawn.
- `sound_chan`, `sound_len` and `chan_offsets`: the sound effects' channels and lengths ((in units of $900 bytes)).
- `nvr_key` and `nvr_magic`: the alphabet used to pack save names, and "ALL" ((any host)).

## Display ((CD-i video))

The game uses both CD-i video planes. Each is an image with a palette ((CLUT)). Plane A is in front ((menus, sprites, signs, cutscenes)) and plane B is behind ((the map, the playfield, backgrounds)).

The display is controlled by tables of instructions:

- A **field control table** for each plane ((`fct_a`, `fct_b`)): instructions run once per frame, such as loading the palette, setting the display mode, and pointing to the line control table.
- **Line control tables** ((`lct_plane_a1/a2/b1/b2` with 480 lines each, the 4-line `lct_head_a/b`, and the 40-line `lct_pal_a/b` on 625-line machines)): instructions for each screen line, mostly "this line's pixels start here" ((`dc_wrli`)). Changing which table a head table links to switches between two buffers ((`flip_A`, `flip_backgrounds`)), and rewriting a table scrolls the picture.

Memory comes from each plane's own video RAM ((`srqcmem`, with colour $80 for plane A and $81 for plane B)):

- `plane_a_mem` ((the first plane A block)) holds plane A's two display buffers and a data area ((`plane_a_data`: the PCB and PCLs, line tables, sprite palette, menu icons, projectile and Zelda graphics)).
- `plane_a_extra` ((a second plane A block of $1FC00 bytes, asked for after the logo)) holds the macro data, the room graphics, the item graphics, the palette file and the sound effects.
- `plane_b_mem` holds plane B's three buffers and the room data at +$43800 ((block map, collision maps, environment projectiles, sprite buffers, the enemy list and records)).

## Loading: PCB and PCL

`ss_play` takes a **PCB** ((play control block, `struct pcb`)). It holds a status, a signal, a record count, channel masks, and the lists of video, audio and data channels.

Each list ((CIL)) holds a **PCL** ((play control list entry, `struct pcl`)) for each channel. A PCL has a control byte, a signal, the next PCL, a buffer address, the buffer size in sectors, an error record and a sector count.

`set_pcb`, `set_pcl` and `reset_pcl` fill them in. The signal handler ((`signal_handler`)) receives the end-of-record and error signals. Cutscenes use three PCLs linked in a ring, so frames keep streaming into three plane A buffers in turn.

## Saved games ((NVRAM))

There are three save slots. A save is a file `/nvr/zelda<n>.nvri` in the player's non-volatile memory ((`save_game_to_NVR`, `load_game_from_NVR`)). `create_file_and_host_names` puts the slot digit into the file name and into the host string "Wand of Gamelon <n>". The info string is "Zelda" ((`save_host_info`)).

The file starts with an 8-byte header ((`struct file_pos`)): a checksum in the low 14 bits of the first word, then the offsets of the host string, the info string and the data. The checksum is the sum of every byte from the host string on. The two strings are packed by `nvr_pack_name` using `nvr_key`. When opening a save `nvr_open` checks that the host string matches ((unless it asks for "ALL")) and that the checksum is right.

The data is the game state, gathered into `mem_space` ((`encode_mem_space`, `decode_mem_space`)): the 32 experience bytes, the low byte of item counts 0 to 20, the rubies as two bytes ((high byte first)), then item counts 22 to 27. A save takes $63 bytes of data and needs $80 bytes more free space than that ((`verify_save_space` checks the slots at startup)). The French, German and Dutch versions check the free space before they delete the old save, the English one deletes first.

## Starting values of the globals

MPW packs the starting values of the globals into a block of data. A routine in code segment 8 unpacks it at startup, before `main` runs. `tools/a5init.py` decodes it, and its description explains the format.

97 of the values are pointers to other globals, which are fixed up at startup: the menu strings, `item_names`, `mkt_demo_ptr`, `voice_ptr`, `demo_ptr`, `bumper_proc`, `bumper_env` and `errno`. A lot of the other tables start out as zero and get filled in at runtime, like `hero_actions`, `macro_ptrs` and the projectile kinds `proj_defs` points to.
