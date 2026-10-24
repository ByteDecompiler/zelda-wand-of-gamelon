/* fix_jump_stick: adjusts the stick event while jumping (?): after the stick has been centred for four
   calls y is set to 8; while y is 8, pushing the stick to one side (x 5-7 or 1-3) turns it into 7 or 1,
   remembered in z so it is not repeated. */

#include "game.h"

void fix_jump_stick(struct stick *stick_ev)
{
    if (stick_ev->x == 0 && stick_ev->count++ == 3) {
        stick_ev->y = 8;
        stick_ev->count = 0;
    } else if (stick_ev->y == 8 && stick_ev->z != 1 && stick_ev->x >= 5 && stick_ev->x <= 7) {
        stick_ev->y = 7;
        stick_ev->z = 7;
    } else if (stick_ev->y == 8 && stick_ev->z != 7 && stick_ev->x >= 1 && stick_ev->x <= 3) {
        stick_ev->y = 1;
        stick_ev->z = 1;
    }
}
