/* restore_life: refills Zelda's life: draws the hearts one by one (with sound 3 and 20 lines'
   wait each) on both plane A buffers. */

#include "game.h"

void restore_life(void)
{
    register long j;
    register long i;
    register long x;
    register long where;
    long half_hearts;
    long y;

    y = 0x20;
    where = 0x18620;    /* the other plane A buffer */
    x = 0;
    damage_taken = 0;
    half_hearts = max_hearts * 2 - damage_taken;
    for (i = 0; i < half_hearts - 1; i += 2) {
        play_sound(3);    /* heart refill sound */
        draw_heart(0, x, y);
        draw_heart(0, x++, where);
        for (j = 0; j < 20; ++j)    /* 20 lines between hearts */
            wait_for_line_event();
    }
    if (i == half_hearts - 1 && i < max_hearts * 2 - 1) {    /* never true: damage_taken was just set to 0 (?) */
        i += 2;
        play_sound(3);
        draw_heart(1, x, y);
        draw_heart(1, x, where);
    }
}
