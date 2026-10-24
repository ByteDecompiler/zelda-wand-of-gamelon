/* jump_off: starts a jump (hero action 5) from a ladder: 1 cell up, rising, ladder climbing
   cleared. */

#include "game.h"

void jump_off(void)
{
    jump_wait_timer = 10;           /* counts as 10 frames since the last jump */
    go_find_hero_action(5);         /* jump */
    zelda_anim_step = 0;
    jump_rising = 1;
    jump_step = 0;
    anim_frame_idx = 0;
    zelda_y -= 1;
    zelda_screen_y -= 1;
    zelda_sprite_y -= 1;
    climbing_blk48 = 0;
    climbing_blk25 = 0;
}
