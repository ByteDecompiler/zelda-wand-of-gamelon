/* init_type_1: starts a type-1 enemy macro (also used by type 3): a sprite that moves back and forth
   between lo and hi; starting in state 6 (left) it heads for lo, otherwise for hi. */

#include "game.h"

void init_type_1(long sprite_index, struct macro_def *def)
{
    register struct macro_def *mdef;

    mdef = def;
    macros[sprite_index]->state = mdef->start_state;
    macros[sprite_index]->lo = mdef->lo;
    macros[sprite_index]->hi = mdef->hi;
    macros[sprite_index]->spread = mdef->spread * 10;
    if (macros[sprite_index]->state == 6)
        macros[sprite_index]->target = macros[sprite_index]->lo;
    else
        macros[sprite_index]->target = macros[sprite_index]->hi;
    init_1_or_4(sprite_index);
}
