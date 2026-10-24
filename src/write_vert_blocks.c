/* write_vert_blocks: writes value into a column of count map blocks from x, y downwards, saving the old
   blocks to save. */

#include "game.h"

void write_vert_blocks(long x, long y, long count, char *save, long value)
{
    register unsigned char *dst;

    dst = block_map + (x + y * map_width);
    while (count--) {
        *save++ = *dst;
        *dst = value;
        dst += map_width;
    }
}
