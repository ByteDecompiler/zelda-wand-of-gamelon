/* nvr_check_name: 1 if the file name ends in "nvri" (a save file), else 0 with errno $3000. */
/* MPW C options: -mbg off */

#include "game.h"

long nvr_check_name(char *name)
{
    register char *ext;
    long len;

    len = strlen(name);
    ext = &name[len - 4];
    if (*ext++ == 'n' && *ext++ == 'v' && *ext++ == 'r' && *ext == 'i')
        return 1;
    errno = 0x3000;
    return 0;
}
