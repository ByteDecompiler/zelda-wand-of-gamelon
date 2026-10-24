/* dec_pf_y: scrolls the playfield up one cell (pf_y in cells, pf_y_px in pixels). */

#include "game.h"

void dec_pf_y(void)
{
    pf_y -= 1;
    pf_y_px -= 4;
}
