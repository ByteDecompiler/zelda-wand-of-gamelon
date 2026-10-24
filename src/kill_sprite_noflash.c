/* kill_sprite_noflash: removes a sprite at once, without the death flash: no sprite, marked gone. */

#include "game.h"

void kill_sprite_noflash(long sprite_idx)
{
    sprite_ptrs[sprite_idx] = 0;
    sprite_gone[sprite_idx] = 1;
}
