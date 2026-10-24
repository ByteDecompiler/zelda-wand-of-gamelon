/* use_rubies: pays `cost` rubies. Returns 1, or 0 (and shows "Not Enough Rubies") if Zelda has too few. */

#include "game.h"

long use_rubies(long cost)
{
    if (rubies < cost) {
        not_enough_rubies = 1;
        return 0;
    }
    rubies -= cost;
    return 1;
}
