/* check_buttons: during the title animation: if a pointer button is down, stops the animation, waits
   until the buttons are released, mutes the sound and returns 1; otherwise returns 0. */

#include "game.h"

long check_buttons(void)
{
    long buttons;
    long x;
    long y;

    pt_coord(video_path, &buttons, &x, &y);
    if (buttons & 3) {                 /* either pointer button */
        ss_abort(anim_path);
        while (buttons & 3)
            pt_coord(video_path, &buttons, &x, &y);
        attenuation(0xff);
        return 1;
    }
    return 0;
}
