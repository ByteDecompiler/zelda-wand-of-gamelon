/* nvr_open: allocates a file record and opens NVRAM file name (nvr_open_into); returns the record, or 0 on
   failure (the record is freed). */
/* MPW C options: -mbg off */

#include "game.h"

struct file_pos *nvr_open(register char *name, char *want, long mode)
{
    register struct file_pos *nvr_file;

    if ((nvr_file = (struct file_pos *)nvr_alloc_mem(0x12, mode)) != 0) {
        if ((nvr_file = nvr_open_into(name, want, mode, nvr_file)) == 0)
            nvr_free_mem(0x12, (char *)nvr_file);
        else
            return nvr_file;
    }
    return 0;
}
