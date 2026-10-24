/* read_sprite_cycle_colors2: copies the sprite palette's second colour-cycle table into cycle_colors. */

#include "game.h"

void read_sprite_cycle_colors2(void)
{
    long i;

    for (i = 0; i < 32; ++i)
        cycle_colors[i] = sprite_palette->cycle2[i];
}
