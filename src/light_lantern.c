/* light_lantern: lights a dark room: the light is largest with the Fairy Lantern (item 2), half that
   with lantern item 1, a quarter with the plain lantern (item 0). */

#include "game.h"

void light_lantern(void)
{
    if (item_counts[2]) {             /* Fairy Lantern */
        lantern_size = 0x500;
        lantern_pos = 0x520 - lantern_pos;
    } else if (item_counts[1]) {      /* Magic Lantern */
        lantern_size = 0x280;
        lantern_pos = 0x2a0 - lantern_pos;
    } else {
        lantern_size = 0x140;
        lantern_pos = 0x160 - lantern_pos;
    }
}
