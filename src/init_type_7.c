/* init_type_7: starts a type-7 sprite (its definition has the type-6 layout): puts 14-cell columns of
   slide blocks 37 (left side) and 86 (right side) into the map at its sides, saving what was there, so
   Zelda slides off it. A definition with state 0 removes the sprite. */

#include "game.h"

void init_type_7(long sprite_index, struct macro_def *def)
{
    register struct macro_def6 *mdef;

    mdef = (struct macro_def6 *)def;
    if (!mdef->state) {
        sprite_ptrs[sprite_index] = 0;
        sprite_drawn[sprite_index] = 0;
        sprite_asleep[sprite_index] = 0;
        return;
    }
    ((struct macro_b *)macros[sprite_index])->macro_no = mdef->state;
    mdef->kind = 2;
    ((struct macro_b *)macros[sprite_index])->frame_offset = mdef->kind;
    write_vert_blocks(sprite_pos[sprite_index][2] - 2, sprite_pos[sprite_index][3] - 13, 14, &((char *)path_blocks)[sprite_index * 18], 0x25);    /* slide left */
    write_vert_blocks(sprite_pos[sprite_index][2] + 2, sprite_pos[sprite_index][3] - 13, 14, &((char *)path_blocks_r)[sprite_index * 18], 0x56);  /* slide right */
    (&anim_frame_idx)[sprite_index] = 0;
    sprite_macros[sprite_index] = macro_ptrs[(unsigned char)(mdef->state - 1)];
    sprite_draw_xs[sprite_index] = sprite_pos[sprite_index][2] - 8;
    ((struct macro_b *)macros[sprite_index])->alt2_macro = 0;
    macros[sprite_index]->state = 2;
    sprite_draw_ys[sprite_index] = sprite_pos[sprite_index][3] - 14;
    (&cur_sprite)[sprite_index] = sprite_frames[sprite_index * 24 + get_image_number(sprite_index)];
    sprite_frame_no[sprite_index] = get_image_number(sprite_index);
}
