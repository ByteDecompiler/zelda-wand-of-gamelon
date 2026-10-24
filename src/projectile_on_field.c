/* projectile_on_field: returns 1 if one of Zelda's projectiles of this type is in flight. */

#include "game.h"

long projectile_on_field(long type)
{
    long i;

    for (i = 0; i < 10; ++i)
        if (projectiles[i].type == type && projectiles[i].state == 1)
            return 1;
    return 0;
}
