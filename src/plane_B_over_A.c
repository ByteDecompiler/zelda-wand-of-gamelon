/* plane_B_over_A: sets plane B in front of plane A (plane order in the FCT). */

#include "game.h"

void plane_B_over_A(void)
{
    dc_wrfi(video_path, fct_a, 5, 0xc2000001);
}
