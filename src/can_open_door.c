/* can_open_door: called with the door block under Zelda and the room number. Returns 1 if
   doors are unlocked (door_open) or check_for_key finds the key for that door; otherwise shows the
   "locked" sign, waits for a button and returns 0. */

#include "game.h"

long can_open_door(long door, long room)
{
    long sign_offset;
    unsigned char ok = 1;

    if (door_open)
        return 1;
    ok = check_for_key(door, room);
    if (!ok) {
        sign_offset = plane_a_buffer * 99840;
        sign_offset = sign_offset + 32;
        bring_up_locked_sign(sign_offset);
        wait_for_button_press_music();
        pointer_buttons = 0;
    }
    return ok;
}
