/* erase_death_number: clears the area where the death screen shows a number (draw_death_number), in
   the plane A buffer at offset y. Note: it passes x_end 131 with x 254, so draw_filled_rect
   fills nothing (?). */

#include "game.h"

void erase_death_number(long y)
{
    draw_filled_rect(DEATH_NUMBER_X, 104, 131, 131, plane_a_mem + y, 1);
}
