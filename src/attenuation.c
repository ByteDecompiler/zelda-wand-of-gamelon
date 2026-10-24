/* attenuation: sets the music volume (CD-i attenuation, 0 = loudest, 255 = silent) on all four channels
   of the audio path; any level below 255 is divided by 10 first, and the level is kept in music_volume. */

#include "game.h"

void attenuation(long level)
{
    music_volume = level;
    if (level < 255)
        level = level / 10;
    sc_atn(audio_path, (level << 24) | (level << 16) | (level << 8) | level);
}
