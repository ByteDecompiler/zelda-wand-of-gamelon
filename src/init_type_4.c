/* init_type_4: sets up a macro of type 4: its state bits start from start_state; bit 0 picks the
   facing (state 6 = left, 2 = right). */

#include "game.h"

void init_type_4(long sprite_no, struct macro_def *def)
{
    register struct macro_def *def_reg;

    def_reg = def;
    macros[sprite_no]->lo = def_reg->start_state;
    macros[sprite_no]->hi = 1;
    if (macros[sprite_no]->lo & macros[sprite_no]->hi)
        macros[sprite_no]->state = 6;
    else
        macros[sprite_no]->state = 2;
    init_1_or_4(sprite_no);
    macros[sprite_no]->hi <<= 1;
}
