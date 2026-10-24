/* set_icf: sets the image contribution factor (brightness 0-63) of plane A (which 0) or plane B
   (which 1) in a line control table. */

#include "game.h"

void set_icf(long plane, long which, long value)
{
    dc_wrli(video_path, plane, 0, 1, (which == 0 ? 0xdb000000 : 0xdc000000) | (value & 0x3f));
}
