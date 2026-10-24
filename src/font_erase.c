/* font_erase: erases text at (x, y) in dest_buf (draw_target is switched for the call and restored). */

#include "game.h"

void font_erase(long x, long y, char *text, long dest_buf)
{
    long saved_target;

    saved_target = draw_target;
    draw_target = dest_buf;
    font_erase_glyphs(x, y, text);
    draw_target = saved_target;
}
