/* play_error: after a load: if it failed, closes zdata.rtr and reopens the next of its three copies and
   returns 1 (retry), up to two times; on the third failure shows the clean-disc sign and goes back to the
   main menu (or quits if no game is running). Returns 0 if the load worked. */

#include "game.h"

long play_error(void)
{
    if (play_failed) {
        if (disc_errors < 2) {
            close(data_path);
            ++disc_errors;
            ++rtr_index;
            if (rtr_index == 3)
                rtr_index = 0;
            if (rtr_index == 0)
                data_path = open(data_file_name0, 1);
            else if (rtr_index == 1)
                data_path = open(data_file_name1, 1);
            else if (rtr_index == 2)
                data_path = open(data_file_name2, 1);
            return 1;
        } else {
            disc_errors = 3;
            clean_disc();
            if (game_started) {
                game_started = 0;
                do_mmenu_code();
            } else
                quit_game(0);
        }
    } else
        return 0;
}
