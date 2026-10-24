/* erase_rope: erases the rope (rows rope_bottom up to 3 above rope_top) from the plane A buffer at offset y,
   the counterpart of draw_rope. */

#include "game.h"

void erase_rope(long y)
{
    long x;
    long top;
    long rel_row;
    long i;

    if (rope_out) {
        x = rope_x - pf_x;
        top = pf_y;
        for (i = rope_bottom; i >= rope_top - 3; --i) {
            rel_row = i - top;
            if (rel_row > 0)
                erase_16x4(x, rel_row, plane_a_mem + y);
        }
    }
}
