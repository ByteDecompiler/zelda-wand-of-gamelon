/* delete_tool: frees a tool slot; if the tool was on screen it is marked (mode 2) to be erased. */

#include "game.h"

struct tool tools[20];

void delete_tool(unsigned char tool_index)
{
    tools[tool_index].state = 0xff;
    if (tools[tool_index].active)
        tools[tool_index].mode = 2;
}
