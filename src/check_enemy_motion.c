/* check_enemy_motion: once a frame, steps every enemy sprite's macro (animation script) when its step
   timer runs out, with the move routine of its macro kind (1/3, 2, 4, 5) or its alternate macro.
   Enemies in state 4 stand still while freeze_timer runs (item 4). */

#include "game.h"

extern unsigned char vulnerability[];   /* -$374c */
extern long enemy_phase;                /* -$1b12 */
void check_sleepers(void);
long move_types_1_and_3(long n, long phase);
long move_type_2(long n);
long move_type_4(long n, long phase);
void do_alternate_macro(long n);
void get_new_image(long n);


void check_enemy_motion(void)
{
    long i;

    check_sleepers();
    zelda_riding = 0;
    for (i = 0; i < 21; ++i)
        /* skip enemies whose record state (+$50) is 4 while freeze_timer (item 4, the Flute) runs */
        if (i && sprite_ptrs[i] && (((char *)(enemy_data + (i - 1) * 0xb0 + 0x50))[0] != 4 || !freeze_timer)) {
            if (macros[i]->alt_active) {
                if (!--macros[i]->step_timer) {
                    macros[i]->step_timer = macros[i]->alt_step_delay;
                    do_alternate_macro(i);
                }
            } else if (!--macros[i]->step_timer) {
                macros[i]->step_timer = macros[i]->step_delay;
                if (macros[i]->kind == 1 || macros[i]->kind == 3) {
                    if (!move_types_1_and_3(i, enemy_phase)) {
                        if ((unsigned char)macros[i]->step_delay > 1)
                            macros[i]->animate = 1;
                        get_new_image(i);
                    }
                } else if (macros[i]->kind == 2) {
                    if (!move_type_2(i)) {
                    }
                } else if (macros[i]->kind == 4) {
                    if (!move_type_4(i, enemy_phase))
                        get_new_image(i);
                } else if (macros[i]->kind == 5)
                    move_type_5(i);
            }
        }
    enemy_phase = 1 - enemy_phase;
}
