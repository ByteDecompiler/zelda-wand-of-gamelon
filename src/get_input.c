/* get_input: returns 1 (after waiting for release) if a pointer button is pressed, else 0. */

#include "game.h"

long get_input(void)
{
    long buttons;
    long x;
    long y;

    pt_coord(video_path, &buttons, &x, &y);
    if (buttons & 3) {
        while (buttons & 3)
            pt_coord(video_path, &buttons, &x, &y);
        return 1;
    }
    return 0;
}
