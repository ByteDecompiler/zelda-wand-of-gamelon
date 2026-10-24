/* get_help_instruction: runs one frame of the world-map help demo: fakes pointer moves and
   button presses from the help_demo script (count, direction bits, buttons in the top bits) and
   handles its voice-over commands $80-$82; any real input ends the demo. */

#include "game.h"

void get_help_instruction(void)
{
    long i;

    if (get_input()) {
        ov_help_repeat = ov_help_cmd = 0;
        set_ov_help_off();
        help_aborted = 1;
    } else if (!voice_wait) {
        voice_ready = 0;
        if (demo_ptr == help_demo) {
            if (map_x + map_cursor_x < 100)  /* first walk the cursor to map spot (100, 80) */
                ++pointer_x;
            else if (map_x + map_cursor_x > 100)
                --pointer_x;
            else
                map_scroll_dx = 0;
            if (map_y + map_cursor_y < 80)
                ++pointer_y;
            else if (map_y + map_cursor_y > 80)
                --pointer_y;
            else
                map_scroll_dy = 0;
            for (i = 0; i < 1000; ++i)    /* a delay loop */
                ;
            if (map_x + map_cursor_x == 100 && map_y + map_cursor_y == 80) {
                ++demo_ptr;
                ov_help_repeat = ov_help_cmd = 0;
            }
        } else if (--ov_help_repeat > 0) {
            if (ov_help_cmd & 8)
                --pointer_y;
            else if (ov_help_cmd & 4)
                ++pointer_y;
            if (ov_help_cmd & 2)
                --pointer_x;
            else if (ov_help_cmd & 1)
                ++pointer_x;
            pointer_buttons = ov_help_cmd >> 5;
        } else {
            ov_help_cmd = *demo_ptr++;
            if (ov_help_cmd == 0xff) {
                set_ov_help_off();
                ov_help_repeat = ov_help_cmd = 0;
            } else if (ov_help_cmd & 0x80) {
                switch (ov_help_cmd) {
                case 0x80:  /* wait for the voice-over, then play the next one */
                    if (!play_done)
                        voice_wait = 1;
                    else {
                        voice_ready = 1;
                        attenuation(0);
                    }
                    pointer_buttons = ov_help_repeat = ov_help_cmd = 0;
                    break;
                case 0x81:  /* voice-over with the help sign on */
                    if (!play_done)
                        voice_wait = 1;
                    else
                        attenuation(0);
                    help_on = 1;
                    break;
                case 0x82:  /* help sign off */
                    help_on = 0;
                    voice_ready = 1;
                    break;
                }
            } else {
                ov_help_repeat = ov_help_cmd;
                ov_help_cmd = *demo_ptr++;
                if (ov_help_cmd & 8)
                    --pointer_y;
                else if (ov_help_cmd & 4)
                    ++pointer_y;
                if (ov_help_cmd & 2)
                    --pointer_x;
                else if (ov_help_cmd & 1)
                    ++pointer_y;
                pointer_buttons = ov_help_cmd >> 5;
            }
        }
    } else if (play_done) {
        voice_wait = 0;
        attenuation(0);
        if (!help_on)
            voice_ready = 1;
    }
}
