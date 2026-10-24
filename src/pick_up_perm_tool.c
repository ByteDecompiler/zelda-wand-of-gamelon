/* pick_up_perm_tool: Zelda touches a permanent item (tool spot index tool): keys (0-17), exits back to
   the world map (18-46; 18-29 also open a new place on the map), or the quest items (47-54). Item 47 (the
   level-2 sword) also restores her life. Returns 1 to remove the item, 0 to keep it. */

#include "game.h"

char pick_up_perm_tool(unsigned char tool)
{
    switch (tool) {
    case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11:
    case 12: case 13: case 14: case 15: case 16: case 17:
        /* a key: experience $81-$92 */
        mark_experience(tool - -0x81);
        break;
    case 18: case 19: case 20: case 21: case 22: case 23: case 24: case 25: case 26: case 27: case 28:
    case 29:
        /* an exit that also opens a new place on the world map */
        open_up_area(tool, 1);
        mark_experience(tool + 0x81);
        demo_started = 1;
        return 0;
    case 30: case 31: case 32: case 33: case 34: case 35: case 36: case 37: case 38: case 39: case 40:
    case 41: case 42: case 43: case 44: case 45: case 46:
        /* an exit back to the world map */
        demo_started = 1;
        return 0;
    case 47:
        sword_level2 = 1;
        if (!hero_killed)
            restore_life();
        tell_what_got(25);
        break;
    case 48:
        item_counts[27] = 1;
        tell_what_got(27);
        break;
    case 49:
        item_counts[19] = 1;
        tell_what_got(19);
        break;
    case 50:
        item_counts[14] = 1;
        tell_what_got(14);
        break;
    case 51:
        item_counts[15] = 1;
        tell_what_got(15);
        break;
    case 52:
        item_counts[17] = 1;
        tell_what_got(17);
        break;
    case 53:
        item_counts[0] = 1;
        tell_what_got(0);
        break;
    case 54:
        item_counts[10] = 1;
        tell_what_got(10);
        break;
    }
    return 1;
}
