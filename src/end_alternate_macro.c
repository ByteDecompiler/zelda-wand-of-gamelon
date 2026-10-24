/* end_alternate_macro: ends a sprite's alternate macro and lets its normal macro resume next
   frame. */

#include "game.h"

void end_alternate_macro(long sprite_no)
{
    macros[sprite_no]->alt_active = 0;
    macros[sprite_no]->step_timer = 1;
    macros[sprite_no]->animate = 0;
}
