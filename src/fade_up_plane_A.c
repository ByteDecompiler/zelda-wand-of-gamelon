/* fade_up_plane_A: brings plane A's image contribution (ICF) up from 0 to 63 on both of its
   line control tables, i.e. fades plane A in. */

#include "game.h"

void fade_up_plane_A(void)
{
    long level;

    for (level = 0; level <= 63; ++level) {
        set_icf(lct_plane_a1, 0, level);
        set_icf(lct_plane_a2, 0, level);
    }
}
