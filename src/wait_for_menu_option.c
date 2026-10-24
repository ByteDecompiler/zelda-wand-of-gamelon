/* wait_for_menu_option: waits 25 frames, then reads the stick into menu_event. */

#include "game.h"

void wait_for_menu_option(void)
{
    long i;

    for (i = 0; i < 25; ++i)
        wait_for_line_event();
    check_stick(&menu_event);
}
