/* get_abs_x: converts a coordinate in one of four forms (which 0-3) to an absolute x: 0 = as is,
   1 = relative to y + 9, 2/3 = relative to y + 9 and mirrored unless facing right/left. No return
   for other values. */

#include "game.h"

long get_abs_x(long which, long x, long y, long dir)
{
    switch (which) {
    case 0:
        return x;
    case 1:
        return y + 9 + x;
    case 2:
        return y + 9 + (dir == 2 ? x : -x);
    case 3:
        return y + 9 + (dir == 6 ? x : -x);
    }
}
