/* tell_what_got: raises the "you got" sign for item. */

#include "game.h"

void tell_what_got(long item)
{
    got_sign_up = 1;
    got_item = item;
}
