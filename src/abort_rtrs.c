/* abort_rtrs: stops any CD play in progress on the three disc files (music, data,
   animations). */

#include "game.h"

void abort_rtrs(void)
{
    ss_abort(music_path);
    ss_abort(data_path);
    ss_abort(anim_path);
}
