/* check_combat_box: check_box (is Zelda inside an enemy's box?), but always 0 while a combat is
   in progress. */

#include "game.h"

long check_combat_box(short *box, long box_dir, long sprite_no, long facing)
{
    if (in_combat)
        return 0;
    return check_box(box, box_dir, sprite_no, facing);
}
