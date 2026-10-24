/* play_animations: plays the 22 animations of anim_sequence one after the other, stopping early when an
   animation is cut short or a pointer button is down, then fades out plane B. Only in the English version. */

#include "game.h"

#ifdef ANIMATION_TESTS

void play_animations(void)
{
    long buttons;
    long x;
    long y;
    long count;
    long i;

    count = 22;
    for (i = 0; i < count; ++i) {
        if (play_animation(anim_sequence[i]))
            break;
        pt_coord(video_path, &buttons, &x, &y);
        if (buttons & 3)
            break;
    }
    fade_out_plane_B();
}

#endif
