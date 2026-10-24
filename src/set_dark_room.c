/* set_dark_room: sets dark_room if this room is in dark_rooms. Without any lantern the light is
   out; with only the plain Lantern, light above 32 is cut to a random 32-95 (whichever is less). */

#include "game.h"

void set_dark_room(void)
{
    long i;
    long random_pos;

    dark_room = 0;
    for (i = 0; i < 15; ++i)
        if (room_number + 1 == dark_rooms[i])    /* dark_rooms holds room numbers + 1 */
            dark_room = 1;
    if (dark_room) {
        if (!item_counts[0] && !item_counts[1] && !item_counts[2])    /* no lantern of any kind */
            lantern_pos = 0;
        else if (!item_counts[1] && !item_counts[2] && item_counts[0] && lantern_pos > 32) {    /* plain Lantern only */
            randomize();
            random_pos = random_seed % 64 + 32;
            lantern_pos = random_pos > lantern_pos ? lantern_pos : random_pos;
        }
    }
}
