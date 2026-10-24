/* init_miss: shows the "miss" puff (projectile type 28, 3 frames) at screen position (x, y). */

#include "game.h"

void init_miss(long x, long y)
{
    init_projectile(28, x + pf_x - 3, y + pf_y - 2, 0, 0, 0, 1, 0, 0, 3, 0);    /* type 28 = miss puff, lives 3 frames */
}
