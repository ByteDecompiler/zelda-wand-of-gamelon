/* draw_life_hearts: draws Zelda's remaining life: a full heart per 2 points of life left
   (max_hearts * 2 - damage_taken), and a half heart for an odd point. */

#include "game.h"

void draw_life_hearts(long y)
{
    long full;
    long heart_slot;
    long i;

    heart_slot = 0;
    full = max_hearts * 2 - damage_taken;
    for (i = 0; i < full - 1; i += 2)
        draw_heart(0, heart_slot++, y);
    if (i == full - 1 && i < max_hearts * 2 - 1) {
        i += 2;
        draw_heart(1, heart_slot++, y);
    }
}
