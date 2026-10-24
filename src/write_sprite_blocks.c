/* write_sprite_blocks: writes `value` into nblocks map blocks from (x, y) to the right, saving the old
   blocks in save; used to make a platform's row solid. */

#include "game.h"

void write_sprite_blocks(long x, long y, long nblocks, char *save, long value)
{
    register unsigned char *dst;

    dst = block_map + (x + y * map_width);
    while (nblocks--) {
        *save++ = *dst;
        *dst++ = value;
    }
}
