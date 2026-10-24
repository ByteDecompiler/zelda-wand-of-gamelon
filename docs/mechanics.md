# Game mechanics

Notes on how *Zelda: The Wand of Gamelon* works, taken from its code. The function names in brackets are where to look for the details. The engine is the same one as *Link: The Faces of Evil* but a lot of the numbers, rooms and items are different so don't trust Link's notes for Zelda.

Units used on this page:

- A **cell** is 4 pixels.
- A **frame** is one step of the game loop. The game waits for a timer signal each step ((`alm_cycle(0x1f9, 0x80000010)` in `drop_sprite`)). That is probably 16 frames a second but this hasn't been confirmed.
- Shot speeds in the code are in 1/8 cells a frame. This page gives them in cells.

## Items

These are the item numbers the code uses. Most names come from the game's own item name table ((`item_names`)). Items 16, 18 and 20 aren't in that table, so their names come from their icons: the game picks an item's icon by its number ((`item_icons[n]`, `load_menu_icons`)), so icon 16 is item 16 and so on.

- 0: Lantern. Burns for $140 frames. Uses a lamp oil.
- 1: Magic Lantern. Burns for $280 frames. Uses a lamp oil.
- 2: Fairy Lantern. Burns for $500 frames and needs no oil. It also shows the invisible flying enemies in rooms 67 to 75.
- 3: Magic Cape. A big jump for 5 rubies. Zelda rises 3, 3, 2 and 2 cells ((10 in all)). She must stand on flat solid ground, not on a slope or a deadly block, and not be climbing or using a rope.
- 4: Flute. Costs 5 rubies. Freezes "type 4" enemies for 80 frames. You can't use it again while they're still frozen.
- 5: Magic Cloak. Costs 20 rubies. For 64 frames Zelda flashes and enemies don't start new attacks on her. Enemy shots still hit though.
- 6: Shroud. If it's selected, the item button fires a lobbed shot ((projectile type 7)) that does 1 damage and isn't used up.
- 7: Bombs. Zelda throws them. Only one bomb can be out at a time. Once it lands the fuse lasts 32 frames.
- 8: Ropes. The rope rises 3 cells a frame. Zelda climbs 1 cell a frame, and 3 for the last stretch. A rope is used up once it catches.
- 9: Power Glove. Costs 10 rubies. After a 5-frame flash, it does 200 damage to every enemy it can hurt within 16 cells left or right and 10 cells up or down.
- 10: Wand of Gamelon. A lobbed shot ((projectile type 5)) that does 1 damage and is never used up. Only one can be in the air at a time.
- 11 and 12: Canteen, full and empty. The full canteen heals Zelda completely and becomes empty. It can only be used when she's hurt.
- 13: Bread. Thrown like a lobbed shot. It does no damage. Where it lands it lies for 240 frames and the flying enemies go for it instead of Zelda ((see below)).
- 14, 15, 17 and 19: Arpagos Feather, Hanyu Spore, Heat Crystal and Arpagos Egg. Quest items, traded for other items.
- 16 and 18: Fairy Dust and the Cloak. Two more quest items. Together they're traded for the Magic Cloak in room 22.
- 14 to 19 are the quest item icons on the subscreen, in two rows of three ((`draw_other_items`)).
- 20: Life Heart. Its icon is the one the subscreen shows next to the number of hearts ((`draw_initial_subscreen`)). The count itself starts at 3 in a new game but nothing reads it, the subscreen shows `max_hearts` instead.
- 21: Rubies. This shares its memory with the `rubies` counter. Up to 999.
- 22: Lamp oil. Up to 99.
- 23: the first sword. Starts at 1. You need it for the plain sword to hurt anything.
- 24 and 25: the level 3 and level 2 sword ((`sword_level3`, `sword_level2`)). The "got" sign for the level 2 sword says Power Sword.
- 26: starts at 1 in a new game and is saved with the other items, but nothing in the code reads it. Like 24, 25 and 27 ((the swords and the shield)) it has no icon, so it was probably meant to hold a value and ended up unused.
- 27: Reflecting Shield ((`shield_level`)).

A new game starts with item 20 at 3, item 23 at 1 and item 26 at 1. Everything else is 0 and all experience bits are cleared ((`initial_item_quantity`)).

The subscreen lists the items in this order, only the ones Zelda has: 8, 7, 13, the best lantern she has, 6, 3, 4, 5, 9, 10 and the canteen ((`fill_tool_list`)).

Lamps ((`light_lantern`, `dim_lantern`, `set_dark_room`)):

- A lantern only works in a dark room, and only when the light is out or down to its last 32 frames.
- The light fades in over 32 frames, stays full, and fades out over the last 32.
- If the plain Lantern is all you have, entering a dark room cuts its light down to a random 32 to 95 frames ((if it had more)).
- Without any lantern, dark rooms stay dark.
- A Peahat ((enemy 70)) hitting Zelda cuts the lantern down to its last 32 frames ((`check_sprite_collisions`)).

Functions: `init_weapon`, `light_lantern`, `big_jump_up`, `power_glove_blast`, `lob_bomb`, `init_bomb`, `init_bomb_hit`, `using_rope`, `lob_shot`, `init_bread`.

## Sword and shots

- A sword thrust does 1 damage. A thrust while standing moves Zelda 4 pixels forward for one frame. It doesn't do this in the air, on a slope, on a moving platform, or on a ladder or rope ((`init_sword_thrust`)).
- The plain sword hurts enemies with bit 8 in their vulnerability byte, and the level 2 sword the ones with bit $10 ((`check_sprite_collisions`)).
- The sword beam fires with the level 3 sword, or with the level 2 sword only when Zelda's health is full. It flies 4 cells a frame ((`init_weapon`, `straight_shot`)).
- The beam starts 1 cell higher than a lobbed shot. It isn't lowered when Zelda crouches, and starts 4 cells higher while she rises in a jump. If a wall is closer than 10 cells it fizzles out.
- A lobbed shot ((Wand, Shroud, Bread)) moves 2.5 cells a frame forward and starts 1.25 cells a frame upward. Gravity pulls it down by 1/4 cell each frame ((`lob_shot`)). It starts 2 cells lower when Zelda is crouching, and 4 cells higher while she rises in a jump. If a wall is within 10 cells it fizzles out.
- A thrown bomb moves 2 cells a frame forward and starts 1.75 cells a frame upward, with the same gravity. Bombs and bread fall at most 3 cells a frame ((`lob_bomb`, `update_projectiles`)).
- Bombs and bread bounce off a wall once, and land on a floor. A bomb then becomes a lit bomb, the bread becomes a loaf lying there ((`check_map`)).
- A bomb does 10 damage in a box around it, and throws 8 pieces of shrapnel. A thrown bomb that hits an enemy blows up right there. A bomb hitting a shield bounces back once ((`init_bomb_hit`, `check_collisions`)).
- One swing hits only one item lying on the floor. Bombs and bread on the floor can only be hit in the shop ((`check_tool_collision`)).

## Zelda

- Life is counted in half hearts. Zelda dies when her damage reaches twice her number of hearts. She starts with 3 hearts, plus one for each heart container found ((experience bits $9F, $A0 and $A1, so 6 at most)) ((`hero_hit`, `set_life_hearts`)).
- Cheat bit 1 makes her invincible: instead of dying her life refills ((`hero_hit`)).
- When an enemy hits her she's knocked back 1 cell a frame for as many cells as that enemy's knockback ((`check_sprite_collisions`, `handle_knockback`)).
- Zelda can only jump again 10 frames after her last jump, and only crouch 10 frames after standing up. After falling for 3 frames she switches to the falling pose. She falls at most 3 cells a frame ((`check_which_upward_motion`, `check_which_downward_motion`, `move_link`)).
- Getting on or off a ladder moves Zelda 3 cells at once. Blocks $56 and $25 slide her right and left, at up to 2 cells a frame ((`climb`, `slide`)).
- Crouching on flat ground and pressing the item button opens the subscreen. Pressing it under a door opens the door, if you have its key ((`drop_sprite`, `can_open_door`)).
- When Zelda dies, the "Continue?" countdown goes from 5 to 0 ((88 frames)). Pressing a button continues in the same room. A continue is only offered for the first two deaths in an area ((`init_death`, `death_loop`, `drop_sprite`)).

## The shop ((room 18, in Sakado))

- 3 bombs cost 20 rubies, 3 ropes cost 10, and 3 lamp oils cost 5. Each count stops at 99. A bought item comes back on the counter 16 frames later ((`sprite_hits_tool`, `buy_item`, `add_fake_*`)).
- If you can't pay in the shop it plays cutscene 3 ((The Merchant Refuses Sale)). For the items that cost rubies elsewhere the game shows "Not Enough Rubies" ((`sprite_hits_tool`, `use_rubies`)).
- Rubies dropped by enemies are worth 1, 5 or 10, up to 999 in total. Touching a lamp oil outside the shop adds 1. Dropped items disappear after 80 frames ((`leave_item`, `more_items`)).
- A heart lying around heals one heart, and a tool of kind 10 heals Zelda fully and fills the canteen if she has it ((`sprite_hits_tool`)).

## Enemies

- Each enemy id has a byte saying which weapons hurt it ((`check_vulnerablility`)): 1 the sword beam, 2 bombs, 4 the Power Glove, 8 the plain sword, $10 the level 2 sword, $20 the Wand shot and $40 the Shroud shot. A shot bounced back by the shield hurts anything. In combat state 7, only the Power Glove and bombs hurt it.
- A beaten enemy loses a life and comes back from its starting state. An enemy with 0 lives in its record comes back forever. One-time enemies stay gone once their experience bit is set ((`check_for_restart`, `init_enemies`)).
- Sleeping enemies wake up after a countdown, when Zelda walks into their wake-up box, or after a delay that starts there. Each try to wake up only works with the record's chance ((in tens of percent)), otherwise it sleeps again ((`check_sleepers`, `wake_on_chance`)).
- Walking enemies pick a random turning point between where they are and their limit ((`get_random_bound`)).
- The flying enemies ((combat type 4)) are the ones the Flute freezes and the Bread lures. While bread lies on the floor they fly at it instead of at Zelda ((`check_enemy_combat`, `check_enemy_motion`)).
- In rooms 67 to 75 ((Reesong Palace)) the flying enemies are invisible unless Zelda has the Fairy Lantern ((`check_if_on_screen`)).
- What enemies drop ((`leave_item`)):
  - kinds 1, 2 and 3: 1, 5 and 10 rubies
  - kind 4: 1 or 5
  - kind 5: 5 or 10
  - kind 6: 10 one time in five, otherwise 1 or 5
  - kind 7: the Wand of Gamelon ((item spot 54))
  - kind 8: the Heat Crystal ((item spot 52))
  - kinds 9 to 20 and 23: a key ((item spots 1, 5 and 7 to 17))
  - kinds 21 and 22: an exit that opens a new place on the map ((item spots 18 and 26))
  - anything else: 10 rubies
- A dropped item that stays ((kinds 7 and up)) gets moved to the room it was dropped in, and its experience bit ($A2 to $B2) is set so it shows up there from now on ((`leave_item`, `check_if_have_tool`)).

## Quest and rooms

- Rewards and trades in each room, when Zelda hits the character there ((sprite 1 unless said otherwise)) ((`start_anim`)):
  - Room 9 ((Shutoy Lake, Lady Alma)): Zelda gets the canteen ((`have_water`)).
  - Room 12 ((Ahitaru, Sanda)): refills Zelda's life and the canteen.
  - Room 16 ((Aru Ainu, Lubonga)): the Shroud, unless she already has it or the Magic Cape.
  - Room 21 ((Sakado, Harbanno)): the Bread.
  - Room 22 ((Sakado, Lika)): the Cloak the first time. Once she has the Fairy Dust, the Fairy Dust and the Cloak are traded for the Magic Cloak.
  - Room 29 ((Sakado, Yokan with the hammer)): the Heat Crystal is traded for the level 3 sword, and her life is refilled.
  - Room 32 ((Kobitan, Makoto)): the Arpagos Egg is traded for the Flute.
  - Room 36 ((Washubi Swamp, Grimbo)): the Hanyu Spore is traded for the Power Glove.
  - Room 40 ((Tykogi Tower, Mayor Cravendish)): sprite 2 gives the Magic Lantern.
  - Room 42 ((Hanyu Forest, Myra)): the Arpagos Feather is traded for the Magic Cape. The Shroud goes too.
  - Room 45 ((The Fairy Pool)): sprites 4 and 5 give a heart container ($9F) and the Fairy Dust the first time, and fill life and the canteen every time.
  - Room 63 ((Nokani Forest, Myra)): the Fairy Lantern.
  - Room 75 ((Reesong Palace, King Harkinian)): the ending ((cutscene 55, Ending Part 1, then `win_game`)).
  - Rooms 4, 13, 18, 24, 37, 38, 40, 50, 55, 57, 60, 64 and 68 just play a cutscene ((38 and 57 also set an experience bit)).
- Beating sprite 1 ((`start_anim`)):
  - Room 39 ((Tykogi Tower, Iron Knuckle)): a heart container ($A0).
  - Room 59 ((Dodomai Palace, Hektan)): a heart container ($A1).
  - Rooms 8, 28, 66 and 74 play a cutscene and set experience bits. Room 73 only sets bit 36.
  - In room 45, beating sprites 1, 2 and 3 plays cutscene 29 ((Witches Die)) and wakes up sprites 4 and 5. Once experience bits 15, 16 and 17 are set they're awake from the start ((`init_enemies`)).
- A shot only counts here if the enemy's combat state is 2 or more. A cutscene started by a bomb waits 10 frames.
- Heart containers refill Zelda's life when their cutscene ends ((`drop_sprite`)).
- Items lying in rooms ((`pick_up_perm_tool`, item spots in `tool_spots`)): the level 2 sword in room 20 ((it also refills her life)), the Reflecting Shield in room 49, the Arpagos Egg in room 26, the Arpagos Feather in room 48, the Hanyu Spore in room 42, the Heat Crystal in room 44 and the Lantern in room 31. The Heat Crystal and the Wand only show up once an enemy dropped them.
- Keys are item spots 0 to 17 ((experience bits $81 to $92)). The spots that an enemy drops only show up after the drop. Key $92 opens two doors in room 53 and room 23 has two locked doors with seperate keys ((`check_for_key`)).
- Item spots 18 to 46 are exits back to the world map. Spots 18 to 29 also open a new place on the map ((`pick_up_perm_tool`)).
- Boss rooms play their boss's cutscene the first time: room 28 Gibdo, 39 Iron Knuckle, 45 the witches and 74 Ganon ((`start_boss_first_anim`)).
- Dark rooms: 6, 19, 28, 33, 38, 44, 45, 51, 53, 54, 62, 63, 69, 70 and 74 ((`set_dark_room`)).
- Room 45 is lit for its boss cutscene ((25, Witches Taunt Zelda)) and then the lantern is blown out, so Zelda starts in the dark ((`drop_sprite`, `start_boss_first_anim`, `play_animation`)).
- In room 8, a shot blocked by the Reflecting Shield bounces back and can hurt any enemy ((`check_collisions`, `reflect_projectile`)).

## Characters and enemies

Every sprite in a room is an "enemy" to the code, the people Zelda talks to included. Each kind has a number ((`enemy_id`)) that picks its graphics, its weapon bits ((`vulnerability`)) and its moves. The rooms are where each one shows up ((from the room enemy lists)).

People:

- 1: Ushrom ((room 18))
- 2: Harbanno ((rooms 21, 38))
- 3: Lika ((room 22))
- 4: Impa ((rooms 4, 24, 50, 55, 64))
- 5: Yokan ((room 13))
- 6: Lubonga ((room 16))
- 7: Makoto ((room 32))
- 8: Grimbo ((room 36))
- 9: Mayor Cravendish ((room 40))
- 10: Sanda ((room 12))
- 11: Yokan ((with the hammer)) ((room 29))
- 12: Myra ((rooms 42, 63))
- 15: Lady Alma ((room 9))
- 16: Duke Onkled ((room 57))
- 17: Lord Kiro ((room 60))
- 18: Impa ((room 37))
- 19: Impa ((room 68))
- 77: King Harkinian ((room 75))

Bosses:

- 21: Gibdo ((room 28))
- 22: Iron Knuckle ((room 39))
- 23: Witch 1 ((room 45))
- 24: Witch 2 ((room 45))
- 25: Witch 3 ((room 45))
- 26: Wizzrobe ((room 8))
- 27: Hektan ((room 59))
- 28: Omfak ((room 66))
- 29: Ganon ((room 74))

Enemies:

- 30: Moblin ((rooms 3, 30, 71))
- 31: Moblin ((kneeling)) ((rooms 2, 7, 19, 30, 31))
- 32: Moblin ((throwing a spear)) ((rooms 2, 30))
- 33: Daira ((rooms 17, 20, 23, 71))
- 34: Daira ((kneeling)) ((rooms 17, 23, 25))
- 36: Wallmaster ((rooms 53, 54))
- 38: Moa ((rooms 62, 63))
- 39: Deadite ((room 65))
- 40: Octorok ((room 37))
- 41: Stalfos ((rooms 28, 47)). The plain one. In room 47 ((the Gobiyan Ship)) they move a different way than in room 28.
- 42: Darknut ((rooms 52, 53, 67))
- 43: Gleeok ((room 44))
- 44: Gleeok head ((rooms 44, 52))
- 45: Dodongo ((rooms 0, 15, 43, 68))
- 46: Armos ((rooms 38, 53, 59))
- 47: Jawbee ((black)) ((rooms 7, 43))
- 48: Jawbee ((red)) ((rooms 7, 43))
- 49: Arpagos, big ((rooms 0, 17, 23, 26)). Purple wings and a bigger hit box.
- 51: Stone head ((room 70))
- 52: no graphics ((a black square)) ((rooms 40, 56))
- 53: Boar ((room 14))
- 54: Ghini ((yellow)) ((rooms 27, 28, 47, 48))
- 55: Ghini ((green)) ((rooms 27, 47, 48))
- 56: Floormaster ((room 38))
- 57: Deeler ((rooms 50, 54, 69, 73))
- 58: Zora ((rooms 1, 46, 51))
- 59: Phyrandaii ((rooms 64, 65, 67))
- 60: Goriya ((room 69))
- 61: Goriya ((kneeling)) ((rooms 61, 69, 73))
- 62: Gohma ((rooms 11, 12))
- 64: Vire ((room 33))
- 65: Keese ((rooms 15, 19, 20, 51, 54, 67))
- 66: Crocodile ((rooms 1, 34, 35, 46, 50, 51))
- 67: Arpagos, small ((rooms 0, 12, 37, 71)). A smaller hit box.
- 70: Peahat ((room 70))
- 71: Stalfos, with the skull pile ((room 27)). Its graphics also hold a pile of skulls that blocks the way; only the Power Glove and bombs break it.
- 72: Spearfish ((room 1))
- 73: Gargoyle ((rooms 41, 42, 61, 68, 70))
- 74: Arpagos, perched ((rooms 34, 35)). Sits until Zelda comes near, then flies; drops nothing.
- 75: Bot ((rooms 41, 42, 61, 62, 63))
- 76: Boon ((room 10))
- 78: Lowder ((rooms 34, 35))

## Places

The 76 rooms belong to 15 places, one for each spot on the world map. A map spot takes Zelda to the first room of its place ((`area_entries`, `main`)), and the doors lead on from there.

- Shutoy Lake: rooms 0 to 9 ((map spot 3, enters room 0))
- Ahitaru: rooms 10 to 13 ((map spot 13, enters room 10))
- Aru Ainu: rooms 14 to 16 ((map spot 0, enters room 14))
- Sakado: rooms 17 to 29 ((map spot 12, enters room 17))
- Kobitan: rooms 30 to 33 ((map spot 11, enters room 30))
- Washubi Swamp: rooms 34 to 36 ((map spot 1, enters room 34))
- Tykogi Tower: rooms 37 to 40 ((map spot 14, enters room 37))
- Hanyu Forest: rooms 41 to 42 ((map spot 4, enters room 41))
- Dordung Cave: rooms 43 to 44 ((map spot 2, enters room 43))
- The Fairy Pool: room 45 ((map spot 9, enters room 45))
- Gobiyan Ship: rooms 46 to 49 ((map spot 5, enters room 46))
- Dodomai Palace: rooms 50 to 60 ((map spot 10, enters room 50))
- Nokani Forest: rooms 61 to 63 ((map spot 7, enters room 61))
- Shrine of Gamelon: rooms 64 to 66 ((map spot 6, enters room 64))
- Reesong Palace: rooms 67 to 75 ((map spot 8, enters room 67))

## Cutscenes

The cutscenes are numbered by their record in `zanim.rtr` ((`game_won` holds the one to play next, `play_animation` plays it)). Some come in a whole and a shortened version: the whole one plays the first time and the shortened one on later visits. Records 1 and 57 to 59 have no sound and aren't listed ((57 and 58 hold the backgrounds of the intro and the ending)).

- 2: The Merchant's Store
- 3: The Merchant Refuses Sale
- 4: Lika's Cloak (Shortened)
- 5: Lubonga's Shroud (Shortened)
- 6: Impa on Tykogi Tower
- 7: Lika's Cloak (Whole)
- 8: Lika's Magic Cloak
- 9: Makoto's Dilemma
- 10: Hungry Girl's Flute
- 11: Lubonga's Shroud
- 12: Gibdo Taunts Zelda
- 13: Gibdo Dies
- 14: Sanda
- 15: Yokan Escapes
- 16: Grimbo's Offer (Whole)
- 17: Grimbo's Power Glove
- 18: Myra's Magic Cape (Shortened)
- 19: Myra's Magic Cape (Whole)
- 20: Harbanno Flees Tykogi Tower
- 21: Iron Knuckle Taunts Zelda
- 22: Iron Knuckle Dies
- 23: Mayor Cravendish's Imprisonment
- 24: Mayor Cravendish's Lantern
- 25: Witches Taunt Zelda
- 26: Harbanno's Bread
- 27: Yokan's New Job (Whole)
- 28: Yokan Tempers The Sword
- 29: Witches Die
- 30: Fairies' Gifts
- 31: Impa on Wizzrobe
- 32: Wizzrobe Dies
- 33: Lady Alma's Canteen
- 34: Impa on the King's Capture
- 35: Impa on Hektan
- 36: Hektan Dies
- 37: Lord Kiro's Advice
- 38: Duke Onkled Surrenders
- 39: Impa on Omfak
- 40: Omfak Dies
- 41: Myra's Lantern
- 42: Impa on Ganon
- 43: Ganon's Retaliation
- 44: Ganon's Defeat
- 45: Impa on Link and the King
- 46: Sanda (Shortened)
- 47: Grimbo's Offer (Shortened)
- 48: Yokan's New Job (Shortened)
- 49: Fairies Heal Zelda
- 50: Tutorial
- 51: Opening Part 1
- 52: Opening Part 2
- 53: Opening Part 3
- 54: Opening Part 4
- 55: Ending Part 1
- 56: Ending Part 2
- 60: Opening Credits
- 61: Credits 1
- 62: Credits 2
- 63: Credits 3
- 64: Credits 4
- 65: Credits 5
- 66: Credits 6
- 67: Credits 7
- 68: Credits 8
- 69: Credits 9

## Things only Zelda has

- Bread ((`init_bread`)): a thrown loaf that lands and lies on the floor as tool kind 18 for $F0 frames. The game keeps its spot in `bread_x`/`bread_y` and the flying enemies aim for it. Only one loaf at a time.
- Moving platforms ((`init_type_5`, `move_type_5`)): enemy type 5 is a platform Zelda can ride. It moves one block a frame between its two limits and turns round at the end or when it's blocked. It carries Zelda with it, and turns her round with it. Its speeds are kept between 1 and 8. On a stop block ($5D) it plays its stop animation, and now and then it switches to another one by chance.
- `restore_arkidai_path` puts back the 12 map blocks a platform covered, before it moves on. The platform writes its own solid blocks into the map where it is, so Zelda can stand on it.
- `blow_out_lantern` sets the lantern's light to 0. It's only used in room 45.
- Link's rafts ((`chk_raft_pos`, `place_raft`)), type 6 movers and the triple shot are gone.
- The English version has two test leftovers: `play_animations` plays the 22 cutscenes listed in `anim_sequence` and `play_all_animations` plays cutscenes 2 to 49, stopping when one is skipped. They're started from map spots 15 and 16 ((spot 17 plays background animation 1)) ((`main`)). The French, German and Dutch versions don't have them.

## Other

- Random numbers: `seed = (seed * 415821 + 1) mod 65536`, and the result is `seed >> 2`. The seed starts from the time of day ((`randomize`, `init_random`)).
- The screen dims after 13500 frames without input in a room or on the subscreen, or 54000 elsewhere ((`screen_dimmer`)). After 1400 frames without input on the title menu, the demo starts ((`mmenu_loop`)).
- The help demo makes sure you have the Lantern, 3 lamp oils, 6 ropes, 3 bombs and 38 rubies, and puts your own items back afterwards ((`do_mmenu_code`)).
- Saves: there are 3 slots in NVRAM, files `/nvr/zelda1.nvri` to `zelda3`. Each save needs $63 bytes of game state plus $80 bytes for the file itself ((`verify_save_space`, `save_game_to_NVR`)). The French, German and Dutch versions check the free space before deleting the old save, the English one deletes it first.
- Any button skips a cutscene ((`play_animation`)).
