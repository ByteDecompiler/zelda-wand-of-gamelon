/* nvr_close_free: closes an NVRAM file and frees its record; 0 on success, -1 on failure. */
/* MPW C options: -mbg off */

#include "game.h"

long nvr_close_free(register struct file_pos *file_rec)
{
    if (nvr_close(file_rec) == 0) {
        nvr_free_mem(0x12, (char *)file_rec);
        return 0;
    }
    return -1;
}
