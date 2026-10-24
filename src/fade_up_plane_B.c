/* fade_up_plane_B: fades plane B in by stepping its ICF from 0 up to 63 in both buffers. */

#include "game.h"

void fade_up_plane_B(void)
{
    long level;

    for (level = 0; level <= 63; ++level) {
        set_icf(lct_plane_b1, 1, level);
        set_icf(lct_plane_b2, 1, level);
    }
}
