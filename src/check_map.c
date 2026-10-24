/* check_map: tests a projectile frame against the room's block map. Projectiles that stop at walls are
   ended with a miss on a solid or wall block (sound 7 for Zelda's, 14 for enemies'); bouncing projectiles
   reverse x once on a wall they move into, and stop on a floor (a lobbed bomb or thrown bread is placed there instead). */

#include "game.h"

void check_map(long proj_idx, long proj_frame)
{
    struct block_info info;
    long bx;
    long by;
    long width;
    long rows;
    register long x;
    register long y;
    long bounced;
    unsigned char *frame;
    struct proj_def *pdef;

    bounced = 0;
    frame = (unsigned char *)proj_cmaps[(short)proj_frame];
    width = frame[0x39];
    if (!width--)
        return;
    bx = (long)frame[0x38] % 18 + projectiles[proj_idx].map_x;
    by = (long)frame[0x38] / 18 + projectiles[proj_idx].map_y;
    rows = 1;
    if (projectiles[proj_idx].type == 25)
        rows = 4; /* a lobbed bomb tests 4 rows */
    if (projectiles[proj_idx].type == 6)
        rows = 4; /* so does thrown bread */
    for (y = 0; y < rows; ++y)
        for (x = 0; x < width + 1; ++x) {
            get_sprite_block(bx + pf_x + x, by + pf_y + y, &info);
            pdef = &proj_defs[projectiles[proj_idx].type];
            if (pdef->stops_at_walls && (info.solid || info.left || info.right)) {
                projectiles[proj_idx].state = 6;
                init_miss(bx + x, by + y);
                if (!pdef->owner) {
                    play_sound(7);
                    return;
                }
                if (pdef->owner == 1)
                    play_sound(14);
                return;
            }
            if (pdef->bounces) {
                if (info.left && projectiles[proj_idx].vx > 0 || info.right && projectiles[proj_idx].vx < 0) {
                    if (!bounced) {
                        projectiles[proj_idx].vx = -projectiles[proj_idx].vx;
                        bounced = 1;
                    }
                } else if (info.floor && projectiles[proj_idx].vy >= 0) {
                    if (projectiles[proj_idx].type == 25) {
                        init_bomb(projectiles[proj_idx].map_x + pf_x + 1, projectiles[proj_idx].map_y + pf_y + y - 1);
                        projectiles[proj_idx].state = 6;
                        return;
                    }
                    if (projectiles[proj_idx].type == 6) {
                        init_bread(projectiles[proj_idx].map_x + pf_x + 1, projectiles[proj_idx].map_y + pf_y + y - 1);
                        projectiles[proj_idx].state = 6;
                        return;
                    }
                    projectiles[proj_idx].vx = 0;
                    projectiles[proj_idx].vy = 0;
                    return;
                }
            }
        }
}
