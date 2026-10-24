/* playfield: plays a room; just calls demo_playfield. No return statement in the game either: main gets
   demo_playfield's result, left in d0. */

#include "game.h"

void playfield(long room, long track, long enemies, long unused, long entry_x, long entry_y, long scroll_x, long scroll_y, unsigned char resume, unsigned char last_room)
{
    demo_playfield(room, track, enemies, unused, entry_x, entry_y, scroll_x, scroll_y, resume, last_room);
}
