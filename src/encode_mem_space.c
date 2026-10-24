/* encode_mem_space: packs the game state into mem_space for a save: the 32 experience bytes, the low
   byte of item counts 0-20, the rubies (item 21) high byte first, then the low bytes of counts 22-27. */

#include "game.h"

char mem_space[61];                 /* the save data, packed (32 experience bytes, item counts) */

void encode_mem_space(void)
{
    register char *out;
    long i;

    out = mem_space;
    for (i = 0; i < 32; ++i)
        *out++ = experiences[i];
    for (i = 0; i < 21; ++i)
        *out++ = item_counts[i];
    *out++ = (rubies & 0xff00) >> 8;      /* rubies is item_counts[21] */
    *out++ = rubies & 0xff;
    for (i = 22; i <= 27; ++i)
        *out++ = item_counts[i];
}
