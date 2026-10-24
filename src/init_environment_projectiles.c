/* init_environment_projectiles: clears all 50 projectile slots, starts the room's environment
   projectiles (traps, up to 30, from env_projectiles) and sets the bounds (in 1/8 cells) beyond
   which projectiles vanish. */

#include "game.h"

void init_environment_projectiles(void)
{
    long i;
    long j;

    for (i = 0; i < 50; ++i)
        projectiles[i].state = 4;
    for (j = 0; j < 30; ++j)
        if (env_projectiles[j * 14]) {
            i = j * 14;
            init_projectile(env_projectiles[i], env_projectiles[i + 1], env_projectiles[i + 2],
                            env_projectiles[i + 3], env_projectiles[i + 4], env_projectiles[i + 5],
                            env_projectiles[i + 6], env_projectiles[i + 7], env_projectiles[i + 8],
                            env_projectiles[i + 9], env_projectiles[i + 10] + 1);
        }
    proj_min_x = -64;    /* bounds in 1/8 cells */
    proj_min_y = -240;
    if (pf_scrolls_horiz == 1) {
        proj_max_x = ((pf_extra_screens + 1) * 96 + 8) * 8;
        proj_max_y = 480;
    } else {
        proj_max_x = 832;
        proj_max_y = (pf_extra_screens + 1) * 60 * 8;
    }
}
