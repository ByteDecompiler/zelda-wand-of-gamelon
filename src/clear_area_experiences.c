/* clear_area_experiences: clears the per-area experience bits 95-126 (when the area has them),
   so that things in the area reset. */

#include "game.h"

void clear_area_experiences(void)
{
    long i;

    if (area_has_exp)
        for (i = 95; i < 127; ++i)
            unmark_experience(i);
}
