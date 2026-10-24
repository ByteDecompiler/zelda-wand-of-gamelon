# Bugs in the original code

Places where the game's code does something the developers probably didn't mean to. They're kept in the decomp since it has to match, but a mod could fix them.

How sure we are about each one:

- **Sure**: the code can't do what it seems meant to do.
- **Likely**: probably a mistake.
- **Maybe**: could be on purpose.

## get_help_instruction ((sure))

The help script's move command moves the pointer down when it should move it right. For direction bit 1 ((right)) it does `++pointer_y` instead of `++pointer_x`. The same test a few lines earlier uses x. You'd only see it in the world map help demo, where the pointer drifts down a bit instead of going right.

## restore_life ((sure))

The half-heart step after the loop can never run. The damage was just set to 0, so the count is always even. You can't see this in the game because life is refilled completely anyway.

## check_sprite_collisions ((sure))

It checks for `result == 2` ((blocked by a shield)), but `get_collision_result` only ever returns 0 or 1, and the shield mask is passed to an argument that is never used. So enemy shields never block Zelda's sword trough this path, and Zelda's shield never blocks an enemy's attack here either.

## erase_death_number ((sure))

It calls `draw_filled_rect(254, 104, 131, 131, ...)`. The right edge ((131)) is left of the left edge ((254)), so nothing is erased. You can't see this, because the new number is drawn over the old one.

## scroll_world ((sure))

It sets `map_moved = 1` and `view_moved = 1` every frame, so the two checks after it are always true. The map and the pointer are redrawn every frame, and the "pointer didn't move" recentring only works the first time.

## check_for_alternate_macro, play_bumper, play_error ((sure))

Some paths in these functions return without a value, so the caller gets whatever is left in register d0. `check_for_alternate_macro` falls off the end for a macro that isn't kind 2 or 3, `play_bumper` never returns anything once the logo has played ((main ignores it)) and `play_error` returns nothing after the third failed read. This happens to work in practice.

## initial_item_quantity ((sure, harmless))

The loop that clears the experience bits runs `i <= 32`, so it clears 33 bytes when there are only 32. The extra byte is the top byte of `button_pressed`, which is always 0 anyway, so nothing happens.

## move_type_5_or_6_1_block ((sure, harmless))

It only sets its two flag pointers ((`f1b` and `f1c`)) for kind 5, but reads them no matter what. This is harmless because its only caller is `move_type_5`, which only runs for kind 5 platforms.

## move_type_5 ((sure, harmless))

When a stop or alt macro runs out of plays it sets `loops = 0x100`, but `loops` is a single byte, so it really stores 0 ((the game does `move.b #$100`)). Since the next decrement wraps 0 round to 255 it still counts 256 plays, and the count doesn't matter once the platform is back on its own macro.

## check_enemy_combat, loop start ((sure, harmless))

The loop starts at `i = 1` and then does `if (!i) continue;`, which can never be true. Its just dead code.

## check_enemy_combat, type 4, state 3 ((likely))

It compares against `cparams[28]` ((the step per frame)), where every other place uses `cparams[26]` ((the target)). A flying enemy's dive would stop at the wrong height.

## check_enemy_combat, type 4, state 1 ((maybe))

It passes `alt_active` ((0 or 1)) as the facing direction to `check_combat_box`, so that attack box is never flipped. The bread check added next to it doesn't change this.

## set_alternate_macro ((maybe))

When the state is neither 2 nor 6, it leaves the image offset unchanged, which could give some alternate animations the wrong facing.

## can_save_restore ((maybe))

The loop runs up to `<= mm_row_count`, one row past the last one, and reads one extra save slot flag.

## init_type_5 ((maybe))

It clamps `speed2` ((byte $1d of the platform's definition)) to 1-8 just like `speed`, but nothing in the game ever reads `speed2`. `move_type_5` sets the step speed when it switches to the alt macro but not when it switches to the stop macro, so `speed2` was maybe meant for that. As it is the stop macro keeps whatever speed the platform had.

## play_all_animations ((maybe))

It plays animations 2 to 49 ((`i` from 1 to 48, playing `i + 1`)), but the game has animations up to 55 ((50 is the help one, 55 the ending)), so the last few are never shown. It's only in the English version as an animation test on a map spot, so it might just be older than those animations.
