/* set_fctA_clut7: puts plane A back in CLUT7 mode (FCT instruction 6) and rewrites its line control table;
   used after an animation. */

#include "game.h"

void set_fctA_clut7(void)
{
    dc_wrfi(video_path, fct_a, 6, 0x78000400);
    reinit_lct_A();
}
