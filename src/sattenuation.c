/* sattenuation: sets the music volume (0 = loudest, 255 = silent; levels below 255 are divided by 12
   for the hardware): left-to-left and right-to-right get the level, the cross channels stay off ($ff). */

#include "game.h"

void sattenuation(long level)
{
    music_volume = level;
    if (level < 0xff)
        level /= 12;
    sc_atn(audio_path, 0xff00ff00 | level << 24 | level << 16 | level << 8 | level);
}
