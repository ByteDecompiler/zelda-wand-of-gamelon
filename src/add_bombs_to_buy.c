/* add_bombs_to_buy: puts the bombs for sale on the shop floor (room 18): a tool of kind 6 at (22, 41) cells.
   Touching it buys 3 bombs for 20 rubies (sprite_hits_tool, buy_item). */

#include "game.h"

void add_bombs_to_buy(void)
{
    add_to_playfield_tools(1, 6, 22, 41, 6, 0);
}
