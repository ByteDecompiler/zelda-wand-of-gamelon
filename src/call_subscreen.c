/* call_subscreen: the item subscreen: draws it, scrolls it on, lets the stick move the
   highlight between items, and on a button press scrolls off and selects the highlighted item.
   Keeps music / voice-over going and the screen dimmer running meanwhile. */

#include "game.h"

char tool_list[27];

void call_subscreen(long where, long buf, long event)
{
    char cur;
    char next;

    cur = 99;  /* 99 = no item */
    next = 99;
    fill_tool_list();
    draw_initial_subscreen(where);
    cur = next = get_first_item(0);
    if (cur != 99)
        highlight_item(cur, where);
    scroll_on_subscreen(where, buf);
    debounce_trig_music();
    debounce_stick();
    for (;;) {
        while (!frame_flag)
            ;
        frame_flag = 0;
        if (music_paused) {
            if (play_done && voice_ready)
                start_voice_over(*voice_ptr++);
        } else if (play_done)
            restart_music();
        check_stick(&event);
        screen_dimmer(event || (pointer_buttons &= 3) != 0, 0x34bc, dark_room ? 2 : 1, 0, 0);
        if (pointer_buttons > 0 || demo_started == 1) {
            scroll_off_subscreen(where, buf);
            if (selected_item != 99)
                selected_item = tool_list[cur];
            reinit_lct_A();
            return;
        }
        if ((next = find_next_item(event, cur)) == 99)
            next = cur;
        if (next != cur) {
            wait_for_line_event();
            if (cur != 99)
                unhighlight_item(cur, where);
            if (next != 99)
                highlight_item(next, where);
            repeat_count = 0;
            while (!frame_flag)
                ;
            frame_flag = 0;
            while (event && ++repeat_count < 8) {  /* auto-repeat delay while the stick is held */
                while (!frame_flag)
                    ;
                frame_flag = 0;
                check_stick(&event);
            }
        }
        cur = next;
    }
}
