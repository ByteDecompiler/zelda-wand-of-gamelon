/* version.h: the values that differ between the versions of cdi_zelda: English (Europe and USA, the
   default), French (-d FRENCH), German (-d GERMAN) and Dutch (-d DUTCH); tools/build.py --version fr|de|nl
   picks the define. Most are disc positions and the layout of text that differs in length. The globals
   also sit at other A5 offsets in each version (config/<version>/symbols.txt), which needs no change to
   the C.
   The layout started from Link: The Faces of Evil's version.h; all values are Zelda's and checked by the
   byte-identical builds. */

#ifndef VERSION_H
#define VERSION_H

/* French, German and Dutch are later builds: their save routine checks the free space before deleting the
   old save (save_game_to_NVR). In Link only German and Dutch did. */
#if defined(FRENCH) || defined(GERMAN) || defined(DUTCH)
#define NEW_SAVE_CHECK
#endif

/* Only the English version still has play_animations and play_all_animations (they play animations in a
   row, maybe for testing), and the map spots 15-17 in main that start them and the ending. */
#if !defined(FRENCH) && !defined(GERMAN) && !defined(DUTCH)
#define ANIMATION_TESTS
#endif

#if defined(FRENCH)

/* where things are in zdata.rtr (sector numbers) */
#define STARTUP_SECTOR      0xb0        /* initial_load: the startup data */
#define ROOM_SECTOR         0xcb        /* load_playfield: room r at r * $c0 + this */
#define EFFECTS_SECTOR      0x3c87      /* load_effects: the sound effects */
#define VOICE_SECTOR        0x394f      /* start_voice_over: clips 0-15 */
#define VOICE_SECTOR_2      0x3afc      /* start_voice_over: clips 16 and up */

/* sizes and offsets of loaded data */
#define PLANE_A_EXTRA_SIZE  0x1fc00     /* post_bumper_video_init */
#define ROOM_PALETTE_OFFSET 0x16c00     /* align_buffers_and_tables: palette after the room graphics */
#define MENU_ICONS_SIZE     0x6400      /* align_buffers_and_tables */
#define FONT_GLYPH_OFFSET   0x5aca      /* load_menu_icons: the font in the menu icon data */
#define CREDITS_LINES       236         /* roll_credits */
#define MAP_CURSOR_START_X  0x100       /* play_background_animation: the map cursor afterwards */

/* the signs drawn with the font: box left and right x, text x */
#define RUBIES_TEXT_X       0x96
#define RUBIES_NUM_X        0x9e
#define LOCKED_BOX_X        0x80
#define LOCKED_BOX_X_END    0x100
#define LOCKED_TEXT_X       0x96
#define CLEAN_BOX_X         0x72
#define CLEAN_BOX_X_END     0x11e
#define CLEAN_TEXT1_X       0x82
#define CLEAN_TEXT2_X       0x98

/* the "Continue?" death screen */
#define DEATH_BOX_X         0x60
#define DEATH_BOX_Y_END     0x94
#define DEATH_LINK_X        0x31        /* Zelda's picture, in cells */
#define DEATH_TEXT_X        0x68
#define DEATH_NUMBER_X      0xfe        /* the countdown digit */
#define DEATH_ERASE_X       200         /* death_loop_bottom: the area cleared before closing */
#define DEATH_ERASE_X_END   264

/* the subscreen (inventory) */
#define ICON_BOX_X          29          /* the left column of item boxes */
#define ICON_BOX_X2         98          /* the right column */
#define ICON_BOX_W          28
#define ICON_COUNT_X        0x3b        /* draw_item_quantity */
#define SELECTED_ICON_X     0xb6        /* the selected item's icon, and the quest items */
#define SELECTED_ICON_X_END 0xcd
#define ITEM_NAME_X         0xd4
#define QUEST_ITEM_STEP     23          /* draw_other_items: between the quest item icons */
#define SWORD_SHIELD_X      0x116       /* draw_sword, draw_shield */
#define BORDER_X            24          /* draw_border: the frame of the item list */
#define BORDER_X_END        0x84
#define BOXES_X             0xb0        /* draw_border: the four boxes on the right */
#define BOXES_X_END         0x13c
#define BOXES_SPLIT         0x52        /* the width of the left one of the two middle boxes */

#elif defined(GERMAN)

#define STARTUP_SECTOR      0xae
#define ROOM_SECTOR         0xc9
#define EFFECTS_SECTOR      0x3d15
#define VOICE_SECTOR        0x394d
#define VOICE_SECTOR_2      0x3b4a

#define PLANE_A_EXTRA_SIZE  0x1fc00
#define ROOM_PALETTE_OFFSET 0x16c00
#define MENU_ICONS_SIZE     0x6400
#define FONT_GLYPH_OFFSET   0x5aca
#define CREDITS_LINES       236
#define MAP_CURSOR_START_X  0xf0

#define RUBIES_TEXT_X       0x90
#define RUBIES_NUM_X        0x9e
#define LOCKED_BOX_X        0x6e
#define LOCKED_BOX_X_END    0x10e
#define LOCKED_TEXT_X       0x84
#define CLEAN_BOX_X         0x7a
#define CLEAN_BOX_X_END     0x116
#define CLEAN_TEXT1_X       0x90
#define CLEAN_TEXT2_X       0x94

#define DEATH_BOX_X         0x58
#define DEATH_BOX_Y_END     0x9c
#define DEATH_LINK_X        0x32
#define DEATH_TEXT_X        0x62
#define DEATH_NUMBER_X      0x102
#define DEATH_ERASE_X       200
#define DEATH_ERASE_X_END   268

#define ICON_BOX_X          31
#define ICON_BOX_X2         106
#define ICON_BOX_W          30
#define ICON_COUNT_X        0x3f
#define SELECTED_ICON_X     0xb8
#define SELECTED_ICON_X_END 0xd1
#define ITEM_NAME_X         0xd6
#define QUEST_ITEM_STEP     25
#define SWORD_SHIELD_X      0x120
#define BORDER_X            26
#define BORDER_X_END        0x8e
#define BOXES_X             0xb2
#define BOXES_X_END         0x14a
#define BOXES_SPLIT         0x5a

#elif defined(DUTCH)

#define STARTUP_SECTOR      0xb0
#define ROOM_SECTOR         0xcb
#define EFFECTS_SECTOR      0x3cc7
#define VOICE_SECTOR        0x394f
#define VOICE_SECTOR_2      0x3b4c

#define PLANE_A_EXTRA_SIZE  0x1fc00
#define ROOM_PALETTE_OFFSET 0x16c00
#define MENU_ICONS_SIZE     0x6400
#define FONT_GLYPH_OFFSET   0x5aca
#define CREDITS_LINES       240
#define MAP_CURSOR_START_X  0xf0

#define RUBIES_TEXT_X       0x90
#define RUBIES_NUM_X        0x9b
#define LOCKED_BOX_X        0x80
#define LOCKED_BOX_X_END    0x100
#define LOCKED_TEXT_X       0x98
#define CLEAN_BOX_X         0x66
#define CLEAN_BOX_X_END     0x102
#define CLEAN_TEXT1_X       0x7c
#define CLEAN_TEXT2_X       0x98

#define DEATH_BOX_X         0x60
#define DEATH_BOX_Y_END     0x94
#define DEATH_LINK_X        0x31
#define DEATH_TEXT_X        0x68
#define DEATH_NUMBER_X      0xfe
#define DEATH_ERASE_X       196
#define DEATH_ERASE_X_END   264

#define ICON_BOX_X          36
#define ICON_BOX_X2         126
#define ICON_BOX_W          35
#define ICON_COUNT_X        0x49
#define SELECTED_ICON_X     0xbd
#define SELECTED_ICON_X_END 0xdb
#define ITEM_NAME_X         0xdb
#define QUEST_ITEM_STEP     30
#define SWORD_SHIELD_X      0x139
#define BORDER_X            31
#define BORDER_X_END        0xa7
#define BOXES_X             0xb7
#define BOXES_X_END         0x16d
#define BOXES_SPLIT         0x6e

#else   /* English */

#define STARTUP_SECTOR      0xaf
#define ROOM_SECTOR         0xc9
#define EFFECTS_SECTOR      0x3ce4
#define VOICE_SECTOR        0x394d
#define VOICE_SECTOR_2      0x3b3a

#define PLANE_A_EXTRA_SIZE  0x1fc00
#define ROOM_PALETTE_OFFSET 0x16c00
#define MENU_ICONS_SIZE     0x5c00
#define FONT_GLYPH_OFFSET   0x5692
#define CREDITS_LINES       240
#define MAP_CURSOR_START_X  0x100

#define RUBIES_TEXT_X       0x8e
#define RUBIES_NUM_X        0x9e
#define LOCKED_BOX_X        0x80
#define LOCKED_BOX_X_END    0x100
#define LOCKED_TEXT_X       0xa0
#define CLEAN_BOX_X         0x80
#define CLEAN_BOX_X_END     0x112
#define CLEAN_TEXT1_X       0x90
#define CLEAN_TEXT2_X       0x9c

#define DEATH_BOX_X         0x60
#define DEATH_BOX_Y_END     0x94
#define DEATH_LINK_X        0x31
#define DEATH_TEXT_X        0x68
#define DEATH_NUMBER_X      0xfe
#define DEATH_ERASE_X       200
#define DEATH_ERASE_X_END   264

#define ICON_BOX_X          29
#define ICON_BOX_X2         98
#define ICON_BOX_W          28
#define ICON_COUNT_X        0x3b
#define SELECTED_ICON_X     0xb6
#define SELECTED_ICON_X_END 0xcd
#define ITEM_NAME_X         0xd4
#define QUEST_ITEM_STEP     23
#define SWORD_SHIELD_X      0x116
#define BORDER_X            24
#define BORDER_X_END        0x84
#define BOXES_X             0xb0
#define BOXES_X_END         0x13c
#define BOXES_SPLIT         0x52

#endif

#endif
