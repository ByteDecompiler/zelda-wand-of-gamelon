/* add_to_tool_list: appends an item to the subscreen's tool list if Zelda has at least
   one of it. */

#include "game.h"

char tool_list[27];

void add_to_tool_list(char tool)
{
    if (item_counts[tool] > 0)
        tool_list[tool_count++] = tool;
}
