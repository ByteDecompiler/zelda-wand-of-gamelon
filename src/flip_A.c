/* flip_A: shows the other of plane A's two buffers (double buffering) by relinking its line
   control table, and toggles plane_a_buffer. */

#include "game.h"

void flip_A(void)
{
    if (plane_a_buffer)
        dc_llnk(video_path, lct_head_a, 2, lct_plane_a1, 0);
    else
        dc_llnk(video_path, lct_head_a, 2, lct_plane_a2, 0);
    plane_a_buffer = 1 - plane_a_buffer;
}
