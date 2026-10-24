/* draw_initial_subscreen: draws the whole item subscreen: border, every item Zelda owns (with
   counts for items 7 and 8), the rubies, heart containers, lamp oil, the quest items, sword and shield. */

#include "game.h"

char tool_list[27];

void draw_initial_subscreen(long where)
{
    char i;

    clear_416x240(where);
    draw_border(where);
    for (i = 0; i < tool_count; ++i)
        if (item_counts[tool_list[i]] > 0) {
            draw_item(i, where);
            switch (tool_list[i]) {
            case 7:
            case 8:
                draw_item_quantity(i, where);
            }
        }
    draw_an_item(where, 21, 3, item_counts[21]);    /* item 21: rubies */
    draw_an_item(where, 20, 2, max_hearts);         /* icon 20 (Life Heart): heart containers */
    draw_an_item(where, 22, 4, item_counts[22]);    /* item 22: lamp oil */
    draw_other_items(where);
    draw_sword(where);
    draw_shield(where);
}
