/* inc_pf_y: moves the playfield scroll position one cell down. */

#include "game.h"

void inc_pf_y(void)
{
    pf_y += 1;
    pf_y_px += 4;
}
