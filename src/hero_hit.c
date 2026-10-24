/* hero_hit: Zelda takes damage (in half hearts); when damage_taken reaches max_hearts * 2 she dies, unless
   cheat bit 1 (invincibility) is on, which just refills her life. */

#include "game.h"

void hero_hit(long damage)
{
    damage_taken += damage;
    if (max_hearts * 2 <= damage_taken) {
        if (!(cheats & 1)) { /* invincibility cheat */
            damage_taken = max_hearts * 2;
            kill_sprite(0);
        } else
            damage_taken = 0;
    }
}
