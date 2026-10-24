/* dim_lantern: one frame of the lantern burning down: lantern_pos counts down to 0 and the light level
   (0-63) follows it: full while pos is between 32 and lantern_size, fading in above lantern_size (just lit)
   and fading out below 32. */

#include "game.h"

void dim_lantern(void)
{
    lantern_pos -= 1;
    if (lantern_pos < 0)
        lantern_pos = 0;
    if (lantern_pos > 31 && lantern_pos <= lantern_size)
        lantern_level = 63;
    else if (lantern_pos == 0)
        lantern_level = 0;
    else if (lantern_pos > lantern_size)
        lantern_level = 63 - (lantern_pos - lantern_size) * 2;
    else
        lantern_level = lantern_pos * 2;
    lantern_level2 = lantern_level;
}
