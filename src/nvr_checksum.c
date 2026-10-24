/* nvr_checksum: sums all bytes of a save file from the host string on (the checksum kept
   in the file header). */
/* MPW C options: -mbg off */

#include "game.h"

long nvr_checksum(register struct file_pos *fp)
{
    unsigned char byte_buf[1];
    long sum = 0;

    lseek(fp->path, fp->host_off, 0);
    while (read(fp->path, (char *)byte_buf, 1))
        sum += byte_buf[0];
    return sum;
}
