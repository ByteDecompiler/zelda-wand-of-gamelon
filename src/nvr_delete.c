/* nvr_delete: deletes a save file; 0 on success, else -1 with errno $3006. */
/* MPW C options: -mbg off */

#include "game.h"

long nvr_delete(register char *name)
{
    if (nvr_check_name(name) && unlink(name) != -1)
        return 0;
    errno = 0x3006;
    return -1;
}
