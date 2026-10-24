/* power_glove_blast: the power glove's blast (end of the glove flash): every enemy that the glove can hurt
   within 16 cells left/right and 10 cells up/down of Zelda takes 200 damage. */

#include "game.h"

void power_glove_blast(void)
{
    register long i;
    register long top;
    register long right;
    register long bottom;
    long left;

    left = zelda_sprite_x + pf_x - 16;
    top = zelda_sprite_y + pf_y - 10;
    right = zelda_sprite_x + pf_x + 16;
    bottom = zelda_sprite_y + pf_y + 10;
    for (i = 1; i < 21; ++i)
        if (sprite_ptrs[i] && check_vulnerablility(i, 0x68)
                && sprite_draw_xs[i] >= left && sprite_draw_xs[i] <= right
                && sprite_draw_ys[i] >= top && sprite_draw_ys[i] <= bottom)
            (&damage_taken)[i] += 200; /* 200 damage */
}
