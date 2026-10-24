/* display_got_sign: draws the "you got" sign for got_item: a framed box sized to the item's two-line name
   (item_names), centred on the screen, into the plane A buffer at offset y. */

#include "game.h"

struct item_name item_names[25];

void display_got_sign(long y)
{
    long line1_width;
    long line2_width;

    line1_width = text_width(item_names[(short)got_item].line1);
    line2_width = text_width(item_names[(short)got_item].line2);
    draw_outlined_box(plane_a_mem + y, got_sign_x = 0xc0 - (line1_width + line2_width + 44) / 2, 100,
                      got_sign_w = (line1_width + line2_width + 44) / 2 + 0xc0, 0x8a, 1, 3);
    font_draw(got_sign_x + 20, 0x68, item_names[(short)got_item].line1, plane_a_mem + y);
    font_draw(got_sign_x + (line1_width + 24), 0x68, item_names[(short)got_item].line2, plane_a_mem + y);
}
