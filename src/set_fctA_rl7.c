/* set_fctA_rl7: switches plane A to RL7 (run-length) coding (from its name, ?) and fills both plane A line
   control tables with one buffer start each (plane_a_buf0, plane_a_buf1). */

#include "game.h"

void set_fctA_rl7(void)
{
    unsigned long i;

    dc_wrfi(video_path, fct_a, 6, 0x78000402);
    lct_a[0] = 0x40000000 | (plane_a_buf0 & 0xffffff);
    for (i = 1; i < 0xf0; ++i)
        lct_a[i] = 0x10000000;
    dc_wrlct(video_path, lct_plane_a1, 0, 0, 0xf0, 1, lct_a);
    lct_a[0] = 0x40000000 | (plane_a_buf1 & 0xffffff);
    dc_wrlct(video_path, lct_plane_a2, 0, 0, 0xf0, 1, lct_a);
}
