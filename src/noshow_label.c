/* noshow_label: removes the place name label shown on the world map. */

#include "game.h"

void noshow_label(void)
{
    restore_label(cur_label);
}
