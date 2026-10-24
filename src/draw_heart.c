/* draw_heart: draws heart number heart_no of the life meter with compiled sprite kind (0 = full,
   1 = half) at line y of plane A. */

#include "game.h"

long heart_x[11];
void (*compiled_sprites[24])(long x, long w, long y, long *lines);

void draw_heart(short kind, short heart_no, long y)
{
    (*compiled_sprites[kind])(heart_x[heart_no] * 4, heart_w * 4, plane_a_mem + y, line_table);
}
