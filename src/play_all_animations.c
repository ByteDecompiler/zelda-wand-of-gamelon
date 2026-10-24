/* play_all_animations: plays animations 2-49 in order, stopping early when one is cut short or a pointer
   button is down, then fades out plane B. Only in the English version (a test leftover?). */

#include "game.h"

#ifdef ANIMATION_TESTS

void play_all_animations(void)
{
    long buttons;
    long x;
    long y;
    long last;
    long first;
    long i;

    last = 49;
    first = 1;
    for (i = first; i < last; ++i) {
        if (play_animation(i + 1))
            break;
        pt_coord(video_path, &buttons, &x, &y);
        if (buttons & 3)
            break;
    }
    fade_out_plane_B();
}

#endif
