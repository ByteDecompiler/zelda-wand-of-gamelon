/* nvr_write: writes to an open NVRAM file at its current position and advances it;
   returns the bytes written, or -1 with errno $3002 (no file) / $3003 (error). */
/* MPW C options: -mbg off */

#include "game.h"

long nvr_write(struct file_pos *fp, char *buf, long write_len)
{
    long got;

    if (fp == 0) {
        errno = 0x3002;
        return -1;
    }
    if (lseek(fp->path, fp->pos, 0) != -1) {
        got = write(fp->path, buf, write_len);
        if (got != -1) {
            fp->pos += got;
            return got;
        }
    }
    errno = 0x3003;
    return -1;
}
