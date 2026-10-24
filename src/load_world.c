/* load_world: loads the world map from zdata.rtr sector 0 (palette file, map image into plane_b_buf0,
   label images into room_gfx), retrying on read errors, then installs the map's palette. */

#include "game.h"

void load_world(void)
{
    long i;

    disc_errors = 0;
    do {
        abort_rtrs();
        set_pcb(0, 0x1f5, 1, 3, 0, (char *)video_cil, 0, (long)data_cil);
        zero_data_cil();
        zero_video_cil();
        ((struct pcl **)pcb->data)[0] = bg_pcl;
        ((struct pcl **)pcb->video)[0] = pcl_b;
        ((struct pcl **)pcb->video)[1] = pcl_c;
        set_pcl(bg_pcl, 0, 0, 0, 0x1f7, 0, (long)palette_file, 0xff, 0, 0);
        set_pcl(pcl_b, 0, 0, 0, 0x1f7, 0, plane_b_buf0, 0xff, 0, 0);
        set_pcl(pcl_c, 0, 0, 0, 0x1f7, 0, room_gfx, 0xff, 0, 0);
        play_done = 0;
        play_failed = 0;
        lseek(data_path, 0, 0);
        ss_play(data_path, pcb);
        while (!play_done)
            ;
    } while (play_error());
    for (i = 0; i < 0x180; ++i)
        palette_out[i] = palette_file[i + 4];
    convert_palette();
}
