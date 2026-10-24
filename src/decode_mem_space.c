/* decode_mem_space: unpacks a loaded save game (mem_space): 32 bytes of experience bits, item
   counts 0-20, the rubies (16 bits), item counts 22-27; then resets the area table and reopens
   the areas whose experience bits ($93-$9e) are set. */

#include "game.h"

char mem_space[61];

void decode_mem_space(void)
{
    register unsigned char *src;
    long i;

    src = (unsigned char *)mem_space;
    initial_item_quantity();
    for (i = 0; i < 32; ++i)
        experiences[i] = *src++;
    for (i = 0; i < 21; ++i)
        item_counts[i] = *src++;
    rubies = *src++ << 8;
    rubies |= *src++;
    for (i = 22; i <= 27; ++i)
        item_counts[i] = *src++;
    selected_item = 99;    /* no item selected */
    initial_areas();
    for (i = 0x93; i <= 0x9e; ++i)    /* area opened experiences -> areas 18-29 */
        if (check_experience(i))
            open_up_area(i - 0x81, 0);
}
