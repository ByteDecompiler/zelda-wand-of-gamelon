/* start_boss_first_anim: when Zelda enters a boss room (boss_flag set), queues the boss's introduction
   cutscene for that room (game_won holds the pending animation number). In room 45 the lantern goes out
   too. */

#include "game.h"

void lantern_brightness();     /* defined without parameters; drop_sprite passes one anyway */

void start_boss_first_anim(long room_no)
{
    if (boss_flag) {
        if (room_no == 28)
            game_won = 12;    /* Gibdo Taunts Zelda */
        else if (room_no == 39)
            game_won = 21;    /* Iron Knuckle Taunts Zelda */
        else if (room_no == 45) {
            game_won = 25;    /* Witches Taunt Zelda */
            blow_out_lantern();
            dim_lantern();
            lantern_brightness();
        } else if (room_no == 74)
            game_won = 43;    /* Ganon's Retaliation */
    }
}
