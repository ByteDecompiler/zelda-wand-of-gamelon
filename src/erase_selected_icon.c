/* erase_selected_icon: clears the "selected item" icon box (24 lines of colour 12) in the buffer
   given (the argument is the draw target, not a colour). */

#include "game.h"

void erase_selected_icon(long color)
{
    long saved;
    long y;

    saved = draw_target;
    draw_target = color;
    for (y = (font_height + 1) / 2 + 28; y <= (font_height + 1) / 2 + 51; ++y)
        draw_line(SELECTED_ICON_X, y, SELECTED_ICON_X_END, y, 12);
    draw_target = saved;
}
