/* retrieve_mkt_demo: gets the next input of the attract-mode demo: a byte with the stick in bits
   0-4 and the buttons in bits 5-7; a button press ends the demo. */

#include "game.h"

void retrieve_mkt_demo(long *x, long *y)
{
    if (get_input()) {
        *x = *y = 0;
        mkt_demo_end = 1;
    } else {
        mkt_demo_byte = *mkt_demo_ptr++;
        *x = mkt_demo_byte & 0x1f;
        *y = mkt_demo_byte >> 5;
    }
}
