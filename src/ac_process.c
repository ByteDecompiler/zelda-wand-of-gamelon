/* ac_process: audio signal handler for the sound effects. While effects are on (music_flags $20), on
   each audio signal it refills the sound map half that just finished: clears it if the same half comes round
   again (and ends a music_wait), steps the current effect channel on by one 0x900-byte block, takes the next
   channel from channel_queue when one ends, and replays the sound map; on an sm_out error effects go off. */

#include "game.h"

long chan_offsets[4];
long smap_saved[2];

long ac_process(long sig)
{
    short side;

    if (sig == audio_signal) {
        if (!(music_flags & 0x20))
            return 0;
        side = !music_side;
        if (side == last_side) {
            memset(smap_saved[side], 0, 0x900);
            if (music_wait) {
                play_done = 1;
                music_wait = 0;
            }
        }
        last_side = side;
        if (!cur_channel) {
            if (channel_queue) {
                chan_pos = 0;
                cur_channel = queue_pop_channel(&channel_queue);
                cur_channel->flags &= ~0x20;
                cur_channel->flags |= 0x10;
            }
        } else if (++chan_pos >= cur_channel->len) {
            cur_channel->flags &= ~0x30;
            cur_channel->next = 0;
            cur_channel = 0;
            if (channel_queue) {
                chan_pos = 0;
                cur_channel = queue_pop_channel(&channel_queue);
                cur_channel->flags &= ~0x20;
                cur_channel->flags |= 0x10;
            }
        }
        if (cur_channel)
            copy_sound(sound_mem[side], cur_channel->data + chan_offsets[chan_pos]);
        sound_pcbs[side].a = 0;
        sound_pcbs[side].b = sound_level;
        if (sm_out(music_path2, sound_ids[side], &sound_pcbs[side]) == -1) {
            music_flags &= ~0x20;
            return -1;
        }
    }
    return 0;
}
