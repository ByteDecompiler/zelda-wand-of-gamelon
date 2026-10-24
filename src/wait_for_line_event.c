/* wait_for_line_event: waits for the next video line event (once per frame). */

#include "game.h"

void wait_for_line_event(void)
{
    line_event = 0;
    while (!line_event)
        ;
}
