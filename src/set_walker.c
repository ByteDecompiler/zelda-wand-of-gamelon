/* set_walker: sends a walking enemy towards map x x: turns it round (state 2 right / 6 left, with its
   walk action) if x is behind it, and sets its target. */

#include "game.h"

void set_walker(long sprite_idx, long x)
{
    if (x < sprite_pos[sprite_idx][2] && macros[sprite_idx]->state == 2) {
        sprite_draw_xs[sprite_idx] = sprite_pos[sprite_idx][2] - 9;
        macros[sprite_idx]->animate = 0;
        macros[sprite_idx]->state = 6;
        go_find_action(sprite_idx, 2);
    } else if (x >= sprite_pos[sprite_idx][2] && macros[sprite_idx]->state == 6) {
        sprite_draw_xs[sprite_idx] = sprite_pos[sprite_idx][2] - 8;
        macros[sprite_idx]->animate = 0;
        macros[sprite_idx]->state = 2;
        go_find_action(sprite_idx, 3);
    }
    macros[sprite_idx]->target = x;
}
