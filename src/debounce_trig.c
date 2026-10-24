/* debounce_trig: waits until both pointer buttons are released, then reads the pointer position (halved
   to screen pixels) into pointer_x/y and old_pointer_x/y. */

#include "game.h"

void debounce_trig(void)
{
    long buttons, x, y;

    while (pt_coord(video_path, &buttons, &x, &y), buttons &= 3)
        ;
    pt_coord(video_path, &buttons, &pointer_x, &pointer_y);
    pointer_x >>= 1;
    pointer_y >>= 1;
    old_pointer_x = pointer_x;
    old_pointer_y = pointer_y;
}
