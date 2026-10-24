/* check_map_row: walks count cells along row y from x = start (either direction) and returns the
   last x before a solid block or a slope facing Zelda's direction. */

#include "game.h"

long check_map_row(long start, long y, long count)
{
    struct block_info info;
    long step;
    long x;

    step = count > 0 ? 1 : -1;
    for (x = start; x != start + count; x += step) {
        get_sprite_block(x, y, &info);
        if (info.solid && !info.left && !info.right)
            break;
        if (info.left && zelda_dir == 2)
            break;
        if (info.right && zelda_dir == 6)
            break;
    }
    return x - step;
}
