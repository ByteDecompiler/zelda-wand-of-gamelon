/* add_fake_bombs: after bombs are bought in the shop (room 18), puts an invisible placeholder (kind 25)
   at the bombs' spot; when its 16-frame timer runs out, update_tools puts the bombs up for sale again. */

#include "game.h"

void add_fake_bombs(void)
{
    add_to_playfield_tools(1, 25, 22, 41, 25, 16);
}
