/* check_map_while_walking: moves Zelda one cell sideways (dir 1 or -1) if the map allows it, and toggles the
   walk animation step so the frame advances every other cell. */

#include "game.h"

void check_map_while_walking(long dir)
{
    move_1_block(0, dir, 0);
    if (!jump_info_solid) {
        if (walk_toggle)
            zelda_anim_step = 1;
        else
            zelda_anim_step = 0;
        walk_toggle = 1 - walk_toggle;
    }
}
