/* draw_item_quantity: prints the count of the item in subscreen slot `slot` (3 digits) next to its icon;
   only the left column (slots 0-4) shows counts. */

#include "game.h"

char tool_list[27];

void draw_item_quantity(char slot, long where)
{
    char buf[4];
    long x;
    long y;

    if (slot < 5) {
        x = ICON_COUNT_X;
        y = slot * 29 + 45;
    } else
        return;
    my_sprintf(buf, 3, item_counts[tool_list[slot]]);
    font_draw(x, y, buf, where);
}
