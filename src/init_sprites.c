/* init_sprites: clears all 21 sprite slots (Zelda and 20 enemies): no sprite, not drawn, not asleep, not
   gone, positions 0. */

#include "game.h"

void init_sprites(void)
{
    long i;

    for (i = 0; i < 21; ++i) {
        sprite_ptrs[i] = 0;
        sprite_drawn[i] = 0;
        sprite_asleep[i] = 0;
        sprite_draw_xs[i] = 0;
        sprite_xs[i] = 0;
        sprite_draw_ys[i] = 0;
        sprite_ys[i] = 0;
        sprite_gone[i] = 0;
    }
}
