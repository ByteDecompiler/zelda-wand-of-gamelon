/* restore_arkidai_path: puts back the 12 map blocks saved under a riding platform's path (sprite_no),
   from its left edge: 8 cells left of it going right, 3 going left. */

#include "game.h"

void restore_arkidai_path(long sprite_no)
{
    macros[sprite_no]->state == 2
        ? restore_sprite_blocks(sprite_pos[sprite_no][2] - 8, sprite_pos[sprite_no][3] - 1, 12, &((char *)path_blocks)[sprite_no * 18])
        : restore_sprite_blocks(sprite_pos[sprite_no][2] - 3, sprite_pos[sprite_no][3] - 1, 12, &((char *)path_blocks)[sprite_no * 18]);
}
