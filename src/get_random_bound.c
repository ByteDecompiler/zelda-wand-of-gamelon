/* get_random_bound: picks where a pacing enemy turns around next: a random x between its
   current position and its lo (walking left, state 6) or hi limit (right); spread is the
   percentage chance of not going all the way (0 = always walk to the limit). */

#include "game.h"

void get_random_bound(long sprite_no, long state)
{
    long range;
    long roll;

    if (state == 6) {
        if (!((struct macro_r *)macros[sprite_no])->spread) {
            ((struct macro_r *)macros[sprite_no])->target = ((struct macro_r *)macros[sprite_no])->lo;
            return;
        }
        range = sprite_pos[sprite_no][2] - ((struct macro_r *)macros[sprite_no])->lo;
        roll = randomize() % (range * 100 / ((struct macro_r *)macros[sprite_no])->spread);
        ((struct macro_r *)macros[sprite_no])->target = ((struct macro_r *)macros[sprite_no])->lo + (roll < range ? roll : 0);
    } else {
        if (!((struct macro_r *)macros[sprite_no])->spread) {
            ((struct macro_r *)macros[sprite_no])->target = ((struct macro_r *)macros[sprite_no])->hi;
            return;
        }
        range = ((struct macro_r *)macros[sprite_no])->hi - sprite_pos[sprite_no][2];
        roll = randomize() % (range * 100 / ((struct macro_r *)macros[sprite_no])->spread);
        ((struct macro_r *)macros[sprite_no])->target = ((struct macro_r *)macros[sprite_no])->hi - (roll < range ? roll : 0);
    }
}
