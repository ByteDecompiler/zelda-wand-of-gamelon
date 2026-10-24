/* restore_vert_blocks: copies saved map blocks back into a vertical column of the block
   map. */

#include "game.h"

void restore_vert_blocks(long x, long y, long block_count, char *src)
{
    register unsigned char *dst;

    dst = block_map + (x + y * map_width);
    while (block_count--) {
        *dst = *src++;
        dst += map_width;
    }
}
