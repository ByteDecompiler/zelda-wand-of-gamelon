/* ac_signal_process: audio signal handler. Signal $131 ends playback; signal $132 marks a
   buffer of the double-buffered music stream done (alternating sides) and ends playback if that
   buffer was flagged as the last; any other signal goes to ac_process. */

#include "game.h"

void ac_signal_process(long sig)
{
    switch (sig) {
    case 0x131:    /* playback finished */
        play_done = 1;
        music_wait = 0;
        break;
    case 0x132:    /* a stream buffer finished */
        if (music_side) {
            if (sig0_flags & 0x80) {
                play_done = 1;
                music_wait = 0;
            }
            sig0_flags = 0;
            sig0_count = 0;
        } else {
            if (sig1_flags & 0x80) {
                play_done = 1;
                music_wait = 0;
            }
            sig1_flags = 0;
            sig1_count = 0;
        }
        music_side = !music_side;
        music_wait = 1;
        break;
    default:
        ac_process(sig);
    }
}
