/* set_pcl: fills a play control list entry (one channel buffer of a CD play). */

#include "game.h"

void set_pcl(struct pcl *pcl, char ctrl, char c2, char c3, short sig, char *nxt, long buf, long buf_size, long err, long cnt)
{
    pcl->ctrl = ctrl;
    pcl->c2 = c2;
    pcl->c3 = c3;
    pcl->sig = sig;
    pcl->nxt = nxt;
    pcl->buf = buf;
    pcl->buf_size = buf_size;
    pcl->err = err;
    pcl->cnt = cnt;
}
