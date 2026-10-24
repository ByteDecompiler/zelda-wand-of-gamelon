/* check_lamp_highlight: if a lamp item (0 Lantern, 1 Magic Lantern or 2 Fairy Lantern) is selected, selects the lamp
   form that is now in the list. */

#include "game.h"

void check_lamp_highlight(char item)
{
    if (selected_item == 2 || selected_item == 1 || selected_item == 0)
        selected_item = item;
}
