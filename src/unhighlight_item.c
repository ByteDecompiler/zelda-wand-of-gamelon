/* unhighlight_item: removes the highlight from a subscreen item: erases its name and the selection frame
   and puts back the icon saved under it. */

#include "game.h"

void unhighlight_item(char item, long where)
{
    erase_item_name(item, where);
    erase_selected_icon(where);
    icon_to_from_save_buffer(item, where, 0);
}
