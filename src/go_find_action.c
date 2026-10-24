/* go_find_action: selects action `id` for sprite `who`: finds its index in that sprite's action table
   (15 entries; 15 if not found). */

#include "game.h"

void go_find_action(long who, long id)
{
    long i;

    for (i = 0; i < 15; ++i)
        if (sprite_actions[who][i].id == id)
            break;
    hero_action_of[who] = i;
}
