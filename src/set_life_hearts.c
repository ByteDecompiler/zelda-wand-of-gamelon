/* set_life_hearts: sets Zelda's heart containers: 3, plus one for each of the three heart experiences
   ($9F-$A1) found. */

#include "game.h"

void set_life_hearts(void)
{
    max_hearts = 3;
    if (check_experience(0x9f))
        max_hearts += 1;
    if (check_experience(0xa0))
        max_hearts += 1;
    if (check_experience(0xa1))
        max_hearts += 1;
}
