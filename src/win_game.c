/* win_game: after the ending: stops the music, plays the background animation and resets the
   game (items, areas, hearts) for a new game. */

#include "game.h"

void win_game(void)
{
    ss_abort(music_path);
    game_won = -1;
    play_background_animation(1);
    game_started = 0;
    door_open = 0;
    initial_item_quantity();
    initial_areas();
    selected_item = 99;
    set_life_hearts();
}
