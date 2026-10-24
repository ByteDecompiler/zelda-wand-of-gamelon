/* nvr_close: closes an NVRAM save file: recomputes its checksum (low 14 bits of the first header word) and
   writes it at the start of the file. Returns 0, or -1 on an error ($3002 for no file). */
/* MPW C options: -mbg off */

#include "game.h"

long nvr_close(register struct file_pos *nvr_file)
{
    if (nvr_file == 0) {
        errno = 0x3002;
        return -1;
    }
    nvr_file->sum = (nvr_file->sum & 0xc000) | (nvr_checksum(nvr_file) & 0x3fff);
    if (lseek(nvr_file->path, 0, 0) != -1) {
        write(nvr_file->path, (char *)&nvr_file->sum, 2);
        if (close(nvr_file->path) != -1)
            return 0;
    }
    return -1;
}
