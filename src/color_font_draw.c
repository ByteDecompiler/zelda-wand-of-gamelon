/* color_font_draw: draws text at (x, y) into dest_buf in the given colour (draw_target is switched for the
   call and restored). */

#include "game.h"

void color_font_draw(long x, long y, char *text, long dest_buf, long color)
{
    long saved_target;

    saved_target = draw_target;
    draw_target = dest_buf;
    color_font_draw_glyphs(x, y, text, color);
    draw_target = saved_target;
}
