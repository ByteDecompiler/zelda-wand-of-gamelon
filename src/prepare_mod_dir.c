/* prepare_mod_dir: loads the modules "ersatz_1" and "ersatz_2" (modcload) and unlinks them again,
   returning errno on a failure. No return at the end in the game either: the caller gets the last result
   left in d0 */

#include "game.h"

long prepare_mod_dir(void)
{
    char *module_a;
    char *module_b;

    if ((module_a = modcload(mod_dir_a, 0, 0x80)) == (char *)-1)
        return errno;
    if ((module_b = modcload(mod_dir_b, 0, 0x80)) == (char *)-1)
        return errno;
    if (munlink(module_a) == -1)
        return errno;
    if (munlink(module_b) == -1)
        return errno;
}
