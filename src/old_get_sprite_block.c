/* old_get_sprite_block: returns the block code at cell (x, y) of the room's block map. */

#include "game.h"

long old_get_sprite_block(long x, long y)
{
    return block_map[x + y * map_width];
}
