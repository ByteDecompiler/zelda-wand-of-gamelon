/* find_next_item: moves the subscreen cursor. The items are in two columns of 5 (slots 0-4 and 5-9):
   up (directions 1, 7, 8) and down (3, 4, 5) move within a column, left (6) and right (2) jump between
   columns. Returns the new slot, or 99 if there is no item there. */

#include "game.h"

char tool_list[27];

char find_next_item(unsigned char dir, char cur)
{
    if (cur == 99)
        return 99;
    switch (dir) {
    case 1:
    case 7:
    case 8:
        if (cur < 5) {
            --cur;
            if (cur == -1)
                cur = 0;
            if (tool_list[cur] == 99)
                return 99;
        } else {
            if (--cur == 4)
                cur = 5;
            if (tool_list[cur] == 99)
                return 99;
        }
        break;
    case 3:
    case 4:
    case 5:
        if (cur < 5) {
            if (++cur == 5)
                cur = 4;
            if (tool_list[cur] == 99)
                return 99;
        } else {
            if (++cur == 10)
                cur = 9;
            if (tool_list[cur] == 99)
                return 99;
        }
        break;
    case 6:
        if (cur < 5 || cur > 9)
            return 99;
        if (tool_list[cur - 5] != 99)
            return cur - 5;
        return 99;
    case 2:
        if (cur < 5) {
            if (tool_list[cur + 5] != 99)
                return cur + 5;
            return 99;
        }
        return 99;
    case 0:
        return 99;
    }
    return cur;
}
