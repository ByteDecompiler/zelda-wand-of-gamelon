/* more_items: adds to an item's count, capped at max. */

#include "game.h"

void more_items(long item, long add, long max)
{
    if (item_counts[item] != max) {
        if (item_counts[item] + add >= max)
            item_counts[item] = max;
        else
            item_counts[item] += add;
    }
}
