/* my_sprintf: writes value as a right-aligned decimal of digits digits (at most 3), with leading zeros
   turned into spaces. */

#include "game.h"

void my_sprintf(char *buf, long digits, long value)
{
    long i;
    register long div;

    buf[digits] = 0;
    div = 100;
    for (i = digits; i < 3; ++i)
        div /= 10;
    for (i = 0; i < digits; ++i) {
        buf[i] = '0';
        while (value >= div) {
            value -= div;
            buf[i] += 1;
        }
        div /= 10;
    }
    for (i = 0; i < digits - 1; ++i) {
        if (buf[i] != '0')
            break;
        buf[i] = ' ';
    }
}
