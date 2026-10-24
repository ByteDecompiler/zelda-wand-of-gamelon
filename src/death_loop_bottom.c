/* death_loop_bottom: clears the death screen's number box (pixels 200-264 x 88-144 of plane A, colour 1),
   saves Zelda's sprite position and erases the death count before it is redrawn with deaths. */

#include "game.h"

void death_loop_bottom(void)
{
    draw_filled_rect(DEATH_ERASE_X, 88, DEATH_ERASE_X_END, 144, plane_a_mem + death_y, 1);
    saved_zelda_y = zelda_sprite_y;
    saved_zelda_x = zelda_sprite_x;
    erase_death_number(death_y);
    deaths_shown = deaths;
}
