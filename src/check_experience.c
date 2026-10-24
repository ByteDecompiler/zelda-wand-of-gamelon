/* check_experience: returns nonzero if experience bit exp_no is set (experiences are
   one-shot flags: items taken, enemies beaten, areas opened, ...). */

#include "game.h"

long check_experience(long exp_no)
{
    long byte, bit;

    byte = exp_no / 8;
    bit = exp_no % 8;
    return experiences[byte] & (1 << bit);
}
