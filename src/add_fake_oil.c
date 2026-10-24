/* add_fake_oil: after lamp oil is bought, puts a display-only oil jar (tool kind 27)
   back on the shop counter at (42, 14); fake shop items cannot be picked up or hit. */

#include "game.h"

void add_fake_oil(void)
{
    add_to_playfield_tools(1, 27, 42, 14, 27, 16);
}
