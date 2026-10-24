/* nvr_read: reads nbytes from an open NVRAM file at its current position and advances it. Returns the
   bytes read, or -1 (errno $3002 no file, $3003 seek failed). */
/* MPW C options: -mbg off */

#include "game.h"

long nvr_read(struct file_pos *fpos, char *buf, long nbytes)
{
    long got;

    if (fpos == 0) {
        errno = 0x3002;
        return -1;
    }
    if (lseek(fpos->path, fpos->pos, 0) != -1) {
        got = read(fpos->path, buf, nbytes);
        fpos->pos += got;
        return got;
    }
    errno = 0x3003;
    return -1;
}
