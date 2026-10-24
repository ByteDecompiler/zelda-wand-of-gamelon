/* highlight_item: draws an item-screen slot highlighted: saves the background, draws the
   highlighted and selected icon and the item's name. */

#include "game.h"

void highlight_item(char item, long where)
{
    icon_to_from_save_buffer(item, where, 1);
    draw_highlighted_icon(item, where);
    draw_selected_icon(item, where);
    draw_item_name(item, where);
}
