/* draw_tools: draws the items lying on the playfield and updates their on-screen state. Items off
   screen are skipped; dropped items (states $66-$68) blink during their last 32 frames; tool
   kinds 25-27 are invisible. */

#include "game.h"

void (*compiled_sprites[24])(long x, long w, long y, long *lines);
struct tool tools[20];

void draw_tools(long where)
{
    long i;
    long x;
    long y;

    for (i = 0; i < 20; ++i) {
        tools[i].draw_x = tools[i].screen_x;
        tools[i].draw_y = tools[i].screen_y;
        tools[i].drawn = tools[i].active;
        x = tools[i].x - pf_x;
        y = tools[i].y - pf_y;
        tools[i].screen_x = tools[i].x - pf_x;
        tools[i].screen_y = tools[i].y - pf_y;
        if (x < -8 || x > 99 || y < 0 || y > 55) {    /* off screen (cells) */
            tools[i].active = 0;
            continue;
        }
        tools[i].active = 1;
        if (tools[i].state == 0xff)    /* unused slot */
            continue;
        if (tools[i].state == 0x66 && tools[i].timer < 32 && (long)tools[i].timer % 2)    /* dropped items blink before vanishing */
            continue;
        if (tools[i].state == 0x68 && tools[i].timer < 32 && (long)tools[i].timer % 2)
            continue;
        if (tools[i].state == 0x67 && tools[i].timer < 32 && (long)tools[i].timer % 2)
            continue;
        if (tools[i].kind == 25 || tools[i].kind == 26 || tools[i].kind == 27)
            continue;
        (*compiled_sprites[(unsigned char)tools[i].kind])(x * 4, y * 4, plane_a_mem + where, line_table);
    }
}
