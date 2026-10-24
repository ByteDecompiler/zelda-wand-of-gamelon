/* init_enemy_projectiles: for each enemy whose combat state is 2 (shooter), uses the last frames of its
   graphics as its projectile images: fills proj_draw/proj_cmaps slots 48+ and the projectile kind's
   left/right animation lists ($FF-terminated); more than 3 frames means separate left-facing frames. */

#include "game.h"

void (*proj_draw[48])();

long proj_cmaps[66];            /* -$14b8: each projectile frame's colour map */

void init_enemy_projectiles(void)
{
    long frame;
    long count;
    struct enemy_st *estate;
    long i;
    long j;
    long extra;
    long slot;

    for (i = 1; i < 21; ++i)
        if (sprite_ptrs[i] || sprite_asleep[i]) {
            estate = (struct enemy_st *)(enemy_data + (i - 1) * 0xb0 + 0x50);
            if (estate->state == 2) { /* combat state 2: shoots */
                extra = 0;
                count = estate->count;
                if (count > 3) {
                    count -= 3;
                    extra = count;
                }
                for (j = 0; j < count; ++j) {
                    slot = (sprite_gfx[i] - 1) * 6 + 48 + j;
                    frame = 24 - count + j - extra;
                    proj_draw[slot] = (void (*)())sprite_frames[i * 24 + frame];
                    proj_cmaps[slot] = cmaps[(short)sprite_gfx[i]] + frame * 0x3a;
                    if (extra > 0) {
                        proj_draw[slot + extra] = (void (*)())sprite_frames[i * 24 + frame + extra];
                        proj_cmaps[slot + extra] = cmaps[(short)sprite_gfx[i]] + (frame + extra) * 0x3a;
                    }
                    ((char *)proj_defs[sprite_gfx[i] + 1].anim_right)[j] = slot;
                    ((char *)proj_defs[sprite_gfx[i] + 1].anim_left)[j] = slot + extra;
                }
                ((char *)proj_defs[sprite_gfx[i] + 1].anim_left)[j] = 0xff;
                ((char *)proj_defs[sprite_gfx[i] + 1].anim_right)[j] = 0xff;
            }
        }
}
