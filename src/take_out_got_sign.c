/* take_out_got_sign: erases the "you got" sign (a box of colour 0 over it) from plane A buffer y. */

#include "game.h"

void take_out_got_sign(long y)
{
    draw_outlined_box(plane_a_mem + y, got_sign_x, 100, got_sign_w, 138, 0, 0);
    got_sign_wait = 0;
}
