/* randomize: steps the random number generator (seed = seed * 415821 + 1, mod 65536) and
   returns the seed >> 2.
   No return statement; the caller gets the value left in d0 */

#include "game.h"

long randomize(void)
{
    register long result;

    result = ((random_seed = (random_seed * 415821 + 1) % 65536) >> 2) % 65536;
}
