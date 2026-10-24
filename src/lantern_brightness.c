/* lantern_brightness: sets plane B's brightness (ICF) to the lantern level, or to full while the
   power glove blast flashes (glove_flash 4 and 2). */

#include "game.h"

void lantern_brightness(void)
{
    if (glove_flash == 4 || glove_flash == 2) {    /* power glove flash frames */
        set_icf(lct_plane_b1, 1, 63);
        set_icf(lct_plane_b2, 1, 63);
    } else {
        set_icf(lct_plane_b1, 1, lantern_level);
        set_icf(lct_plane_b2, 1, lantern_level);
    }
}
