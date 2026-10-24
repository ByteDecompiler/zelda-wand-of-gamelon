/* start_anim: picks the cutscene (game_won = animation number) for an enemy or character that was
   killed or hit (kind = projectile type, 99 = the sword) in room, and applies its reward: heart
   containers, sword levels, Water of Life, item trades, experience bits.
   An old-style definition, so that the char parameter is loaded into its register first
   (the prototype in game.h says long, which a char parameter promotes to) */

#include "game.h"

void start_anim(room, sprite_no, kind)
long room;
long sprite_no;
char kind;
{
    if (kind != 99    /* projectiles only count if the record's byte $50 >= 2 */
            && ((unsigned char *)(enemy_data + (sprite_no - 1) * 0xb0 + 0x50))[0] < 2)
        return;
    if (!sprite_ptrs[sprite_no]) {
        if (room == 45 && (sprite_no == 1 || sprite_no == 2 || sprite_no == 3)
                && !sprite_ptrs[1] && !sprite_ptrs[2] && !sprite_ptrs[3]) {    /* all three gone */
            game_won = 29;    /* Witches Die */
            anim_flag_239c = 1;
            return;
        }
        if (sprite_no != 1)
            return;
        if (room == 28)
            game_won = 13;    /* Gibdo Dies */
        else if (room == 39) {    /* heart container $a0 once */
            if (!check_experience(0xa0)) {
                mark_experience(0xa0);
                heart_bonus = 1;
                anim_restores_life = 1;
            }
            game_won = 22;    /* Iron Knuckle Dies */
        } else if (room == 8) {
            game_won = 32;    /* Wizzrobe Dies */
            mark_experience(32);
        } else if (room == 59) {    /* heart container $a1 once */
            if (!check_experience(0xa1)) {
                mark_experience(0xa1);
                heart_bonus = 1;
                anim_restores_life = 1;
            }
            game_won = 36;    /* Hektan Dies */
            mark_experience(33);
            mark_experience(34);
        } else if (room == 66) {
            game_won = 40;    /* Omfak Dies */
            mark_experience(35);
        } else if (room == 74) {
            game_won = 44;    /* Ganon's Defeat */
            mark_experience(23);
            mark_experience(33);
            mark_experience(34);
        } else if (room == 73)
            mark_experience(36);
        return;
    }
    if (sprite_no == 1) {
        if (room == 24)
            game_won = 45;    /* Impa on Link and the King */
        else if (room == 37)
            game_won = 6;    /* Impa on Tykogi Tower */
        else if (room == 4)
            game_won = 31;    /* Impa on Wizzrobe */
        else if (room == 50)
            game_won = 34;    /* Impa on the King's Capture */
        else if (room == 55)
            game_won = 35;    /* Impa on Hektan */
        else if (room == 64)
            game_won = 39;    /* Impa on Omfak */
        else if (room == 68)
            game_won = 42;    /* Impa on Ganon */
        else if (room == 18)
            game_won = 2;    /* The Merchant's Store */
        else if (room == 60) {
            game_won = 37;    /* Lord Kiro's Advice */
            kill_sprite_noflash(1);
        } else if (room == 57) {
            game_won = 38;    /* Duke Onkled Surrenders */
            kill_sprite_noflash(1);
            mark_experience(24);
        } else if (room == 9) {
            game_won = 33;    /* Lady Alma's Canteen */
            kill_sprite_noflash(1);
            have_water = 1;
        } else if (room == 22) {
            if (item_counts[16]) {    /* the Fairy Dust (16) and the Cloak (18) traded for the Magic Cloak (5) */
                game_won = 8;    /* Lika's Magic Cloak */
                item_counts[18] = 0;
                item_counts[16] = 0;
                item_counts[5] = 1;
                kill_sprite_noflash(1);
            } else if (item_counts[18])
                game_won = 4;    /* Lika's Cloak (Shortened) */
            else {
                item_counts[18] = 1;
                game_won = 7;    /* Lika's Cloak (Whole) */
            }
        } else if (room == 16) {
            if (item_counts[6] || item_counts[3])
                game_won = 5;    /* Lubonga's Shroud (Shortened) */
            else {
                item_counts[6] = 1;
                game_won = 11;    /* Lubonga's Shroud */
            }
        } else if (room == 21) {
            game_won = 26;    /* Harbanno's Bread */
            item_counts[13] = 1;
        } else if (room == 32) {
            if (item_counts[19]) {    /* the Arpagos Egg (19) traded for the Flute (4) */
                item_counts[4] = 1;
                item_counts[19] = 0;
                game_won = 10;    /* Hungry Girl's Flute */
                kill_sprite_noflash(1);
            } else
                game_won = 9;    /* Makoto's Dilemma */
        } else if (room == 29) {
            if (item_counts[17]) {    /* the Heat Crystal (17) -> sword level 3 */
                sword_level3 = 1;
                item_counts[17] = 0;
                anim_restores_life = 1;
                game_won = 28;    /* Yokan Tempers The Sword */
            } else if (sword_level3)
                game_won = 48;    /* Yokan's New Job (Shortened) */
            else
                game_won = 27;    /* Yokan's New Job (Whole) */
        } else if (room == 13) {
            game_won = 15;    /* Yokan Escapes */
            kill_sprite_noflash(1);
        } else if (room == 36) {
            if (item_counts[15]) {    /* the Hanyu Spore (15) traded for the Power Glove (9) */
                game_won = 17;    /* Grimbo's Power Glove */
                item_counts[15] = 0;
                item_counts[9] = 1;
            } else if (item_counts[9])
                game_won = 47;    /* Grimbo's Offer (Shortened) */
            else
                game_won = 16;    /* Grimbo's Offer (Whole) */
        } else if (room == 75)
            game_won = 55;    /* Ending Part 1 */
        else if (room == 38) {
            game_won = 20;    /* Harbanno Flees Tykogi Tower */
            kill_sprite_noflash(1);
            mark_experience(31);
        } else if (room == 40)
            game_won = 23;    /* Mayor Cravendish's Imprisonment */
        else if (room == 12) {
            if (check_experience(6))
                game_won = 46;    /* Sanda (Shortened) */
            else
                game_won = 14;    /* Sanda */
            get_water_of_life();
            anim_restores_life = 1;
        } else if (room == 63) {
            game_won = 41;    /* Myra's Lantern */
            item_counts[2] = 1;
            kill_sprite_noflash(1);
        } else if (room == 42) {
            if (item_counts[14]) {    /* the Arpagos Feather (14) traded for the Magic Cape (3); the Shroud (6) goes too */
                item_counts[3] = 1;
                item_counts[14] = 0;
                item_counts[6] = 0;
                game_won = 19;    /* Myra's Magic Cape (Whole) */
                kill_sprite_noflash(1);
            } else
                game_won = 18;    /* Myra's Magic Cape (Shortened) */
        }
    } else if (room == 45 && (sprite_no == 4 || sprite_no == 5)) {    /* first time: heart $9f and the Fairy Dust (16) */
        if (!check_experience(0x9f)) {
            mark_experience(0x9f);
            heart_bonus = 1;
            get_water_of_life();
            anim_restores_life = 1;
            item_counts[16] = 1;
            game_won = 30;    /* Fairies' Gifts */
        } else {
            get_water_of_life();
            anim_restores_life = 1;
            game_won = 49;    /* Fairies Heal Zelda */
        }
    } else if (sprite_no == 2 && room == 40) {
        game_won = 24;    /* Mayor Cravendish's Lantern */
        item_counts[1] = 1;
        kill_sprite_noflash(1);
    }
    if (game_won != -1 && kind == 25)    /* killed by a bomb: wait 10 frames */
        anim_delay = 10;
}
