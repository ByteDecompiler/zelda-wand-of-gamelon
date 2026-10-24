/* wait_got_sign: keeps the "you got" sign up until a button is pressed (got_sign_wait), then clears the
   button state. */

#include "game.h"

void wait_got_sign(void)
{
    got_sign_wait = 1;
    got_sign_up = 0;
    wait_for_button_press_music();
    pointer_buttons = 0;
}
