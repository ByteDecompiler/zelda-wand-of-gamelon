/* explode_bomb: a placed bomb's fuse ran out: removes the tool, plays the explosion (sound 8) and starts the
   blast at the bomb (init_bomb_hit, not a direct hit). */

#include "game.h"

struct tool tools[20];

void explode_bomb(unsigned char tool_idx)
{
    tools[tool_idx].state = 0xff;
    bomb_placed = 0;
    play_sound(8);
    init_bomb_hit(tools[tool_idx].x + 2, tools[tool_idx].y + 2, 0);
}
