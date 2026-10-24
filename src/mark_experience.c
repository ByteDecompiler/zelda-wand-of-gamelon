/* mark_experience: sets experience bit exp_id (byte exp_id / 8, bit exp_id % 8): something done or found
   that the game remembers (and saves). */

#include "game.h"

void mark_experience(long exp_id)
{
    long byte, bit;

    byte = exp_id / 8;
    bit = exp_id % 8;
    experiences[byte] |= 1 << bit;
}
