/* set_pcb: fills the play control block for the next ss_play. */

#include "game.h"

void set_pcb(short stat, short sig, long rec, long chan, short achan, char *video, char *audio, long data)
{
    pcb->stat = stat;
    pcb->sig = sig;
    pcb->rec = rec;
    pcb->chan = chan;
    pcb->achan = achan;
    pcb->video = video;
    pcb->audio = audio;
    pcb->data = data;
}
