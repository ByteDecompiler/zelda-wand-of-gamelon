/* add_fake_ropes: after ropes are bought in the shop, puts a placeholder rope tool (kind 26, state 126)
   at cell (57, 14) with a 16-frame timer; when it runs out update_tools calls add_ropes_to_buy. */

#include "game.h"

void add_fake_ropes(void)
{
    add_to_playfield_tools(1, 26, 57, 14, 26, 16);
}
