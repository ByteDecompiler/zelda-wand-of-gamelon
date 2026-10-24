/* check_which_downward_motion: Zelda pressed down: on top of a ladder (blocks 48/25) she starts
   climbing down it (3 cells down, facing right); otherwise she crouches, but only 10 or more frames
   after she last stood up. */

#include "game.h"

void check_which_downward_motion(struct motion *motion_ptr)
{
    check_for_climb(1);
    if (climbing_blk48 || climbing_blk25) {
        if (climbing_blk48)
            go_find_hero_action(13);
        if (climbing_blk25)
            go_find_hero_action(14);
        old_zelda_dir = zelda_dir;
        zelda_dir = 2;
        zelda_sprite_x = zelda_screen_x - 8;
        anim_frame_idx = 0;
        zelda_y += 3;
        zelda_screen_y += 3;
        zelda_sprite_y += 3;
        zelda_moved = 1;
        motion_ptr->next = 4;
    } else if (crouch_wait_timer >= 10) {    /* 10 frames since standing up */
        zelda_crouching = 1;
        crouch_wait_timer = 10;
    }
}
