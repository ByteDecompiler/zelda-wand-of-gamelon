/* debounce_stick: reads the pointer; if it is outside (20..364, 20..220) it is put back at the
   centre (192, 120). Remembers it as the old position and clears menu_event. */

#include "game.h"

void debounce_stick(void)
{
    long buttons;

    pt_coord(video_path, &buttons, &pointer_x, &pointer_y);
    pointer_x >>= 1;    /* to 384x240 coordinates */
    pointer_y >>= 1;
    menu_event = 0;
    if (pointer_x > 0x16c || pointer_x < 0x14 || pointer_y > 0xdc || pointer_y < 0x14) {
        pointer_x = 0xc0;
        pointer_y = 0x78;
        pt_pos(video_path, pointer_x << 1, pointer_y << 1);
    }
    old_pointer_x = pointer_x;
    old_pointer_y = pointer_y;
}
