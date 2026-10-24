/* buy_item: buys 3 of an item in the shop for cost rubies, capped at max, and restocks the shelf.
   Returns 3 if the player already has max, 4 if there are not enough rubies, 2 when bought. */

#include "game.h"

long buy_item(long item, long cost, long max)
{
    if (item_counts[item] == max)
        return 3;
    if (rubies < cost)
        return 4;
    item_counts[item] += 3;
    if (item_counts[item] > max)
        item_counts[item] = max;
    rubies -= cost;
    switch (item) {
    case 7:                         /* bombs */
        add_fake_bombs();
        break;
    case 8:                         /* ropes */
        add_fake_ropes();
        break;
    case 22:                        /* lamp oil */
        add_fake_oil();
        break;
    }
    return 2;
}
