/* init_map: shows the world map: points plane B's line control table at the map image (704 pixels wide) at the
   current scroll position (map_x, map_y). */

#include "game.h"

void init_map(void)
{
    plane_b_buffer = 1;
    move_map();
}
