/* check_canteen_highlight: if the canteen (item 11 full / 12 empty) is selected, selects
   whichever canteen form is now in the list. */

#include "game.h"

void check_canteen_highlight(char item)
{
    if (selected_item == 11 || selected_item == 12)
        selected_item = item;
}
