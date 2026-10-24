/* add_ropes_to_buy: puts the ropes for sale on the shop floor (room 18): a tool of kind 8 at (57, 14) cells.
   Touching it buys 3 ropes for 10 rubies (sprite_hits_tool, buy_item). */

#include "game.h"

void add_ropes_to_buy(void)
{
    add_to_playfield_tools(1, 8, 57, 14, 8, 0);
}
