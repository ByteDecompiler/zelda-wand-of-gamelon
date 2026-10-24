/* init_projectile: starts a projectile in a free slot of its owner's range (0-9 Zelda's,
   10-19 enemies', 20-49 the room's). Velocities are in 1/8 cells a frame; it can start after a
   delay and repeat at an interval, and lives life_frames (0 = until it leaves). */

#include "game.h"

void init_projectile(long type, long x, long y, long vx, long vy, long accel_y, long num_repeats, long start_delay, long fire_interval, long life_frames, long hit_damage)
{
    long first;
    long last;
    long i;
    long k;

    if (!proj_defs[type].owner) {
        first = 0;
        last = 10;
    } else if (proj_defs[type].owner == 1) {
        first = 10;
        last = 20;
    } else {
        first = 20;
        last = 50;
    }
    for (i = first; i < last; ++i)
        if (projectiles[i].state == 4)  /* 4 = free slot */
            break;
    if (i >= last)
        return;
    projectiles[i].type = type;
    if (!start_delay) {
        projectiles[i].state = 1;
        projectiles[i].wait = fire_interval;
    } else {
        projectiles[i].state = 0;
        projectiles[i].wait = start_delay;
    }
    projectiles[i].map_x = projectiles[i].x = x - pf_x;
    projectiles[i].map_y = projectiles[i].y = y - pf_y;
    projectiles[i].start_x = x - vx / 8;
    projectiles[i].start_y = y - vy / 8;
    projectiles[i].fx = projectiles[i].start_x * 8;
    projectiles[i].fy = projectiles[i].start_y * 8;
    projectiles[i].vx = projectiles[i].start_vx = vx;
    projectiles[i].vy = projectiles[i].start_vy = vy;
    projectiles[i].gravity = accel_y;
    if (projectiles[i].vx >= 0)
        projectiles[i].anim = (unsigned char *)proj_defs[type].anim_right;
    else
        projectiles[i].anim = (unsigned char *)proj_defs[type].anim_left;
    for (k = 0; projectiles[i].anim[k + 1] != 0xff; ++k)
        ;
    projectiles[i].frame = k;
    projectiles[i].shown = 0;
    projectiles[i].visible = 0;
    projectiles[i].repeats = num_repeats;
    projectiles[i].interval = fire_interval;
    projectiles[i].lifetime = life_frames;
    projectiles[i].life = projectiles[i].lifetime;
    projectiles[i].damage = hit_damage;
}
