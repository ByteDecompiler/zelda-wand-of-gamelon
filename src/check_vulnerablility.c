/* check_vulnerablility: nonzero if the weapon (a projectile type or a melee code) hurts enemy sprite_idx.
   An enemy in combat state 7 is hurt only by the power glove ($68) and bombs ($19); otherwise the enemy's
   vulnerability[] bits decide. Weapon 1 (a projectile reflected by the shield) hurts everything. */

#include "game.h"

extern unsigned char vulnerability[];   /* -$374c */
extern long enemy_phase;                /* -$1b12 */

long check_vulnerablility(long sprite_idx, long weapon)
{
    if (((char *)(enemy_data + (sprite_idx - 1) * 0xb0 + 0x50))[0] == 7)
        return weapon == 0x68 || weapon == 0x19;
    switch (weapon) {
    case 0x6c: /* plain sword (needs item 23, the first sword) */
        return vulnerability[macros[sprite_idx]->enemy_id - 1] & 8;
    case 0x74: /* sword with sword_level2 */
        return vulnerability[macros[sprite_idx]->enemy_id - 1] & 0x10;
    case 0x68: /* power glove */
        return vulnerability[macros[sprite_idx]->enemy_id - 1] & 4;
    case 0: /* sword beam */
        return vulnerability[macros[sprite_idx]->enemy_id - 1] & 1;
    case 1: /* reflected projectile */
        return 1;
    case 0x19: /* bomb */
        return vulnerability[macros[sprite_idx]->enemy_id - 1] & 2;
    case 5:
        return vulnerability[macros[sprite_idx]->enemy_id - 1] & 0x20;
    case 7:
        return vulnerability[macros[sprite_idx]->enemy_id - 1] & 0x40;
    }
    return 0;
}
