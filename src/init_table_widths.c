/* init_table_widths: fills line_table with the start of each of the 240 screen lines (416 bytes a line). */

#include "game.h"

void init_table_widths(void)
{
    long i;

    for (i = 0; i < 240; ++i)
        line_table[i] = i * 416;
}
