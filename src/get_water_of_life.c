/* get_water_of_life: refills Zelda's life fully, and fills the canteen if she has the Water of Life
   (water_used = 1 means full; the empty canteen, item 12, becomes the full one, item 11). */

#include "game.h"

void get_water_of_life(void)
{
    if (have_water) {
        water_used = 1;    /* canteen full */
        if (selected_item == 12)
            selected_item = 11;
    }
    damage_taken = 0;
}
