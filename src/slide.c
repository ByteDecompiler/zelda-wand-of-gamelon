/* slide: slides Zelda down a slippery slope (move_link: block $56 slides right, $25
   left; also used by big_jump_up): up to two cells sideways a frame, stepping down where the
   ground falls away, until she reaches a floor, ladder or track. wall_flag points to the jump_info
   field (left/right) that marks a wall. */

#include "game.h"

void slide(long dx, long wall_flag)
{
    long i;

    for (i = 0; i < 2; ++i) {
        get_sprite_block(zelda_x + dx, zelda_y, &jump_info);
        if (jump_info.floor || jump_info.climb_a || jump_info.climb_b || jump_info.deadly || jump_info.track || *(long *)wall_flag)
            set_link_position(dx);
        else {
            get_sprite_block(zelda_x + dx, zelda_y + 1, &jump_info);
            if (jump_info.floor || jump_info.climb_a || jump_info.climb_b || jump_info.deadly || jump_info.track || *(long *)wall_flag) {
                set_link_position(dx);
                ++zelda_y;
                ++zelda_screen_y;
                ++zelda_sprite_y;
            } else {
                get_sprite_block(zelda_x, zelda_y + 1, &jump_info);
                if (*(long *)wall_flag) {
                    ++zelda_y;
                    ++zelda_screen_y;
                    ++zelda_sprite_y;
                } else {
                    set_link_position(dx);
                    ++zelda_y;
                    ++zelda_screen_y;
                    ++zelda_sprite_y;
                }
            }
        }
        zelda_anim_step = 0;
        if (jump_info.floor || jump_info.track
                || jump_info.climb_a && (old_get_sprite_block(zelda_x, zelda_y + 1) != 48
                                    || old_get_sprite_block(zelda_x, zelda_y - 1) != 48)
                || jump_info.climb_b && (old_get_sprite_block(zelda_x, zelda_y + 1) != 25
                                    || old_get_sprite_block(zelda_x, zelda_y - 1) != 25)) {
            if (!zelda_thrusting) {
                go_find_hero_action(0);
                anim_frame_idx = 0;
            }
            break;
        }
    }
}
