/* get_collision_result: scans one row of a 16-bit collision mask between bit low_bit and high_bit (from
   the top or the bottom end, mirrored when flip) and reports the first set bit: *hit = 1 and its
   position in *pos. Only ever reports 0 or 1. */

#include "game.h"

void get_collision_result(long high_bit, long low_bit, unsigned short *mask, long unused, long down, long flip,
                          long *hit, long *pos)
{
    long i;

    *pos = 0;
    *hit = 0;
    if (down) {
        if (flip) {
            for (i = high_bit; i >= low_bit; --i)
                if (*mask & bit_masks[15 - i]) {
                    *hit = 1;
                    *pos = 15 - i;
                    return;
                }
        } else {
            for (i = high_bit; i >= low_bit; --i)
                if (*mask & bit_masks[i]) {
                    *hit = 1;
                    *pos = 15 - i;
                    return;
                }
        }
    } else {
        if (flip) {
            for (i = low_bit; i <= high_bit; ++i)
                if (*mask & bit_masks[15 - i]) {
                    *hit = 1;
                    *pos = 15 - i;
                    return;
                }
        } else {
            for (i = low_bit; i <= high_bit; ++i)
                if (*mask & bit_masks[i]) {
                    *hit = 1;
                    *pos = 15 - i;
                    return;
                }
        }
    }
}
