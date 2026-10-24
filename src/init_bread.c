/* init_bread: puts a loaf of bread (tool kind 18) down at (x, y) for $f0 frames and remembers where it is,
   so enemies can go for it. */

#include "game.h"

void init_bread(long x, long y)
{
    add_to_playfield_tools(1, 18, x, y, 18, 0xf0);
    bread_timer = 0xf0;
    bread_x = x;
    bread_y = y;
}
