/* jump_up: one frame of the rising phase of a jump: moves Zelda up jump_heights[jump_step] cells (stopping
   at a ceiling, which starts the fall) and sideways if the stick says so. After 4 steps the fall begins. */

#include "game.h"

long jump_heights[5];

void jump_up(struct motion *move_info)
{
    long i;

    move_info->dir = move_info->next;
    for (i = 1; i <= jump_heights[jump_step]; ++i) {
        get_sprite_block(zelda_x, zelda_y - 1, &jump_info);
        if (jump_info_solid) {
            jump_rising = 0;
            jump_falling = 1;
            anim_frame_idx = 1;
            break;
        }
        --zelda_y;
        --zelda_screen_y;
        --zelda_sprite_y;
        zelda_moved = 1;
    }
    if (move_info->dir == 2 || move_info->dir == 1)  /* right or up-right */
        jump_up_dir(1);
    else if (move_info->dir == 6 || move_info->dir == 7)  /* left or up-left */
        jump_up_dir(-1);
    if (jump_rising) {
        ++jump_step;
        if (jump_step == 4) {  /* top of the jump: start falling */
            jump_rising = 0;
            jump_falling = 1;
            --jump_step;
            anim_frame_idx = 1;
        }
    }
}
