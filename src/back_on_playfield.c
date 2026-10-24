/* back_on_playfield: restores the playfield display after a subscreen or sign: rebuilds both
   planes' line control tables and fades them back up; in a dark room plane B only comes up to
   the lantern's brightness. */

#include "game.h"

void back_on_playfield(void)
{
    long level;

    reinit_lct_A();
    reinit_lct_B();
    if (dark_room) {
        dim_lantern();
        for (level = 0; level <= 63; ++level) {
            set_icf(lct_plane_a1, 0, level);
            set_icf(lct_plane_a2, 0, level);
            if (level <= lantern_level) {
                set_icf(lct_plane_b1, 1, level);
                set_icf(lct_plane_b2, 1, level);
            }
        }
    } else
        fade_up_plane_AB();
}
