/* check_screen_edges: moves Zelda one cell at a time towards her target screen position and scrolls the
   playfield when she passes the scroll margins (y > 45 or < 22, x > 52 or < 44) and there is room to
   scroll; past the screen edges she is pushed back. Returns 1 if anything moved. */

#include "game.h"

long check_screen_edges(void)
{
    register long steps;
    long delta;
    long moved;

    moved = 0;
    steps = zelda_target_y - zelda_screen_y;
    if (steps < 0)
        steps = -steps;
    for (; steps > 0; --steps) {
        if (zelda_screen_y > 45 && pf_y_px_max - 3 > pf_y_px) {
            --zelda_screen_y;
            --zelda_sprite_y;
            inc_pf_y();
            moved = 1;
        } else if (zelda_screen_y + 0 > 59) {
            --zelda_y;
            --zelda_screen_y;
            --zelda_sprite_y;
            moved = 1;
        } else if (zelda_screen_y < 22 && pf_y_px > 3) {
            ++zelda_screen_y;
            ++zelda_sprite_y;
            dec_pf_y();
            moved = 1;
        } else if (zelda_screen_y - 14 < 0) {
            delta = zelda_target_x - zelda_screen_x;
            zelda_x += delta;
            zelda_screen_x += delta;
            zelda_sprite_x += delta;
            delta = zelda_target_y - zelda_screen_y;
            zelda_y += delta;
            zelda_screen_y += delta;
            zelda_sprite_y += delta;
        }
    }
    steps = zelda_target_x - zelda_screen_x;
    if (steps < 0)
        steps = -steps;
    for (; steps > 0; --steps) {
        if (zelda_screen_x > 52 && pf_x_px_max - 3 > pf_x_px) {
            --zelda_screen_x;
            --zelda_sprite_x;
            inc_pf_x();
            moved = 1;
        } else if (zelda_screen_x + 9 > 103) {
            --zelda_x;
            --zelda_screen_x;
            --zelda_sprite_x;
            moved = 1;
        } else if (zelda_screen_x < 44 && pf_x_px > 3) {
            ++zelda_screen_x;
            ++zelda_sprite_x;
            dec_pf_x();
            moved = 1;
        } else if (zelda_screen_x - 9 < -8) {
            ++zelda_x;
            ++zelda_screen_x;
            ++zelda_sprite_x;
            moved = 1;
        }
    }
    return moved;
}
