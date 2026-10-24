/* screen_dimmer: the screen saver: after count idle frames fades plane A (and plane B: mode 1 always,
   mode 2 not past the lantern level) and optionally the sound down; on input (on set) fades back up and
   resets the idle count. */

#include "game.h"

void screen_dimmer(long on, long count, long mode, long sound, long stereo)
{
    long i;

    if (on) {
        if (idle_frames >= count) {
            gc_col(video_path, 0x80008080);
            for (i = 21; i <= 63; i += 2) {
                wait_for_line_event();
                set_icf(lct_plane_a1, 0, i);
                set_icf(lct_plane_a2, 0, i);
                if (mode == 1 || mode == 2 && i <= lantern_level) {
                    set_icf(lct_plane_b1, 1, i);
                    set_icf(lct_plane_b2, 1, i);
                }
                if (sound) {
                    if (stereo)
                        sattenuation((63 - i) * 2);
                    else
                        attenuation((63 - i) * 2);
                }
            }
        }
        idle_frames = 0;
    } else if (idle_frames == count) {
        for (i = 63; i > 20; i -= 2) {
            wait_for_line_event();
            set_icf(lct_plane_a1, 0, i);
            set_icf(lct_plane_a2, 0, i);
            if (mode == 1 || mode == 2 && i <= lantern_level) {
                set_icf(lct_plane_b1, 1, i);
                set_icf(lct_plane_b2, 1, i);
            }
            if (sound) {
                if (stereo)
                    sattenuation((63 - i) * 2);
                else
                    attenuation((63 - i) * 2);
            }
        }
        gc_col(video_path, 0x80000000);
        ++idle_frames;
    } else if (idle_frames < count)
        ++idle_frames;
}
