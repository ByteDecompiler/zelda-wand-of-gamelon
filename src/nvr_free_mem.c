/* nvr_free_mem: returns memory to the system (srtmem). */
/* MPW C options: -mbg off */

#include "game.h"

long nvr_free_mem(long ret_size, char *ret_mem)
{
    long result;

    if ((result = srtmem(ret_size, ret_mem)) == -1) {
        /* empty in the game too: the test is compiled, nothing follows it */
    }
    return result;
}
