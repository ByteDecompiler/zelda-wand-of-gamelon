/* game.h - declarations shared by the decompiled cdi_zelda sources.
   Function names are the game's own (MacsBug names), except the NVRAM file routines (nvr_*), which have
   none. Globals, struct fields and library calls are named from what the code does with them; rename
   freely, and keep config/symbols.txt in step: the build resolves globals and library calls by these
   names. The comment after each global is its A5 offset, then what it holds.
   Units: a cell is 4 pixels (positions are mostly in cells); Zelda's direction is 2 = right, 6 = left.
   docs/data.md describes the data the game reads and its tables. */

#ifndef GAME_H
#define GAME_H

#include "version.h"   /* values that differ between the game's versions */

struct tool {                       /* 20 bytes, array at -$c64 */
    short x;                        /* +$00 */
    short y;                        /* +$02 */
    short screen_x;                 /* +$04 */
    short screen_y;                 /* +$06 */
    short draw_x;                   /* +$08 */
    short draw_y;                   /* +$0a */
    char kind;                      /* +$0c */
    unsigned char state;            /* +$0d */
    char active;                    /* +$0e */
    char drawn;                     /* +$0f */
    char mode;                      /* +$10 */
    char pad2;
    unsigned short timer;           /* +$12 */
};

struct projectile {                 /* 40 bytes */
    unsigned char type;             /* +$00 */
    char state;                     /* +$01 */
    short map_x;                    /* +$02 */
    short map_y;                    /* +$04 */
    short x;                        /* +$06 */
    short y;                        /* +$08 */
    short start_x;                  /* +$0a */
    short start_y;                  /* +$0c */
    short fx;                       /* +$0e */
    short fy;                       /* +$10 */
    short vx;                       /* +$12 */
    short vy;                       /* +$14 */
    short start_vx;                 /* +$16 */
    short start_vy;                 /* +$18 */
    unsigned char gravity;          /* +$1a */
    unsigned char frame;            /* +$1b */
    unsigned char *anim;            /* +$1c */
    char shown;                     /* +$20 */
    char visible;                   /* +$21 */
    char repeats;                   /* +$22 */
    char interval;                  /* +$23 */
    char wait;                      /* +$24 */
    char lifetime;                  /* +$25 */
    char life;                      /* +$26 */
    unsigned char damage;           /* +$27 */
};

struct pcl {                        /* CD-i play control list entry (PCL): one channel's buffer of a play */
    char ctrl;                      /* +$00 */
    char pad0;
    char c2;                        /* +$02 */
    char c3;                        /* +$03 */
    short sig;                      /* +$04 */
    char *nxt;                      /* +$06 */
    long buf;                       /* +$0a */
    long buf_size;                  /* +$0e */
    long err;                       /* +$12 */
    short pad1;
    long cnt;                       /* +$18 */
};

struct menu_icons {                 /* at -$1122 */
    long pad[14];
    long sword1;                    /* +$38 */
    long sword2;                    /* +$3c */
    long sword3;                    /* +$40 */
    long small_shield;              /* +$44 */
    long big_shield;                /* +$48 */
};

/* Globals */
extern long video_path;                                                     /* -$2250 */
extern long fct_a;                                                          /* -$2258: plane A field control table id (dc_crfct) */
extern long line_event;                                                     /* -$24ee */
extern long pf_y;                                                           /* -$24ea */
extern long pf_y_px;                                                        /* -$24da */
extern long pf_x;                                                           /* -$24e6 */
extern long pf_x_px;                                                        /* -$24d6 */
extern long *lct_b;                                                         /* -$2298: plane B line control table image, one entry per playfield line */
extern long cur_label;                                                      /* -$15c8: label shown now, -1 = none */
extern long plane_b_buffer;                                                 /* -$15b4: which plane B LCT (b1 when 1, b2 when 0) gets the next background; toggled on each flip */
extern long plane_b_buf0;                                                   /* -$2238: plane B buffer 0: the map or playfield, 384 bytes a line */
extern long map_y;                                                          /* -$2c2e */
extern long map_x;                                                          /* -$2c2a */
extern char mkt_demo;                                                       /* -$3778 */
extern long zelda_x;                                                         /* -$1e3a */
extern long zelda_screen_x;                                                  /* -$1e42: Zelda's x on the playfield screen (zelda_x - pf_x), in cells; = sprite_pos[0][0] */
extern long zelda_sprite_x;                                                  /* -$2020: Zelda's sprite draw x (zelda_screen_x - 8 or - 9); = sprite_draw_xs[0] */
extern char zelda_moved;                                                     /* -$1e23 */
extern char jump_info_solid;                                                /* -$1df1: jump_info.solid (+$11) under another name; also set to block a jump */
extern long walk_toggle;                                                    /* -$19f6 */
extern char zelda_anim_step;                                                 /* -$1e22: set when zelda_moved: advance anim_frame_idx; toggled so frames advance every other step */
extern char zelda_thrusting;                                                 /* -$1e28: 1 while a sword thrust or item-use pose is in progress */
extern char thrust_lunge;                                                   /* -$1e21: 1 = this thrust may lunge forward 4 cells (not when riding, airborne or on a slope) */
extern char continuing;                                                     /* -$37fe */
extern long anim_frame_idx;                                                 /* -$1d12: index into the current action's frame list ((&anim_frame_idx)[n] for sprite n) */
extern long disc_errors;                                                    /* -$2c16: read retries of the current load (0-2); 3 = the disc can't be read */
extern char selected_item;                                                  /* -$2812 */
extern short have_water;                                                    /* -$2bc6 */
extern short water_used;                                                    /* -$2bc8 */
extern long damage_taken;                                                   /* -$27de */
extern char bomb_placed;                                                    /* -$1ec4 */
extern long *data_cil;                                                      /* -$2bee */
extern char area_has_exp;                                                   /* -$1930 */
extern char nvr_name[];                                                     /* -$191a */
extern unsigned short rubies;                                               /* -$2bb4 */
extern char not_enough_rubies;                                              /* -$2800 */
extern long pointer_x;                                                      /* -$23c8: pointer x (pt_coord, halved) */
extern long pointer_y;                                                      /* -$23cc: pointer y */
extern long old_pointer_x;                                                  /* -$23d0: pointer x of the last frame */
extern long old_pointer_y;                                                  /* -$23d4: pointer y of the last frame */
extern unsigned char music_volume;                                          /* -$1ec0 */
extern long music_path;                                                     /* -$2c22 */
extern long menu_event;                                                     /* -$1de2 */
extern char in_combat;                                                      /* -$27f8 */
extern short shield_level;                                                  /* -$2ba8 */
extern struct menu_icons menu_icons;                                        /* -$1122 */
extern unsigned long random_seed;                                           /* -$23e4 */
extern struct projectile *projectiles;                                      /* -$14c0 */
extern struct tool tools[];                                                 /* -$c64 */
extern long *line_table;                                                    /* -$22b0: offset of each of the 240 screen lines */
extern long plane_a_mem;                                                    /* -$2224: start of plane A's memory (0x47420 bytes, srqcmem colour 0x80) */
extern long heart_w;                                                        /* -$ad4 */
extern long heart_x[];                                                      /* -$ad0 */
extern void (*compiled_sprites[])(long x, long w, long y, long *lines);     /* -$e08: code pointers, filled by load_tool_imagery */
extern long door_open;                                                      /* -$15cc */
extern long plane_a_buffer;                                                 /* -$2284: which of plane A's two buffers is shown (flip_A) */
extern long pointer_buttons;                                                /* -$23c4: pointer buttons (pt_coord); 1 and 2 are the two buttons */
extern char got_sign_up;                                                    /* -$2802 */
extern char got_sign_wait;                                                  /* -$27fe */
extern char rubies_sign_wait;                                               /* -$27fc */
extern long got_item;                                                       /* -$1132 */
extern long fct_b;                                                          /* -$225c: plane B field control table id (dc_crfct) */
extern long palette_in[];                                                   /* -$2b9c: the palette as CLUT load commands */
extern unsigned char palette_out[];                                         /* -$2994 */
extern long anim_path;                                                      /* -$2c26: path of zanim.rtr (animations) */
extern long data_path;                                                      /* -$2c1e: path of zdata.rtr (world, rooms, sounds); reopened on one of the three copies after errors */
extern long *video_cil;                                                     /* -$2be6 */

/* Game functions */
void restore_label(long label_id);
void move_map(void);
void play_overworld_music(void);
long add_to_playfield_tools(unsigned char kind, char sub, unsigned short x, unsigned short y, char tool_kind, short timer);
void unmark_experience(long exp_no);
void go_find_hero_action(char action);
void abort_rtrs(void);
void flip_world_lcts(long *lct);
long check_box(short *box, long dir, long sprite_idx, long state);
void move_1_block(long sprite_idx, long dx, long stop_at_edge);
void bring_up_locked_sign(long n);

void fade_out_plane_A(void);
void wait_for_line_event(void);
void sattenuation(long n);
void check_stick(long *event);
unsigned char check_for_key(long door, long room);
void fade_up_plane_AB(void);
void reinit_lct_A(void);
void install_palette(long plane, char *pal, char *table);
void wait_for_button_press_music(void);

extern char anim_file_name[];           /* -$1962: "zanim.rtr" */
extern char data_file_name[];           /* -$196c: "zdata.rtr" */
extern char initial_area_tab[];         /* -$15dc */
extern char areas[];                    /* -$2e46 */
extern long draw_target;                /* -$24d2: buffer that draw_line and the font routines draw into */
extern unsigned short item_counts[];    /* -$2bde */
extern char *sprite_ptrs[];             /* -$1f24 */
extern char sprite_gone[];              /* -$21f0 */
extern char sprite_killed[];            /* -$21da */
extern char hero_killed;                /* -$27f6 */
extern unsigned char *block_map;        /* -$229c */
extern long map_width;                  /* -$152e */
extern long got_sign_x;                 /* -$112a */
extern long got_sign_w;                 /* -$112e */
extern long game_won;                   /* -$280a */
extern char game_started;               /* -$192c: a game is in progress */
extern long menu_icon_tab[];            /* -$1122: the same table as menu_icons */
void init_rtr(void);
void initial_areas(void);
void draw_filled_rect(long x, long y, long x_end, long y_end, long buf_base, unsigned char color);
void draw_outlined_box(long where, long x1, long y1, long x2, long y2, long fill, long border);
void play_background_animation(long n);
void font_draw_glyphs(long x, long y, char *text);  /* assembly, gap_01A90 */
void font_erase_glyphs(long x, long y, char *text); /* assembly, gap_01A90+$160 */
long get_input(void);
void initial_item_quantity(void);
void set_life_hearts(void);

struct hero_action {                /* 6 bytes, array at -$1c6c */
    char id;
    char pad;
    short *frames;                  /* +$02 */
};

extern long font_height;                                                /* -$1a20 */
extern long font_width;                                                 /* -$1a1c */
extern long font_bitmap_a;                                              /* -$1a18 */
extern long font_bitmap_b;                                              /* -$1a14 */
extern long font_bitmap_c;                                              /* -$1a10 */
extern unsigned char experiences[];                                     /* -$2220: (bits) */
extern char locked_sign_done;                                           /* -$27fa */
extern char tool_list[];                                                /* -$105a */
extern long tool_count;                                                 /* -$105e */
extern struct hero_action hero_actions[];                               /* -$1ca8 */
extern unsigned char hero_action;                                       /* -$1cbe */
extern char jump_wait_timer;                                            /* -$1e29: frames since the last jump; a jump needs >= 10 */
extern char jump_rising;                                                /* -$1e2c: 1 while Zelda is in the rising phase of a jump */
extern char jump_step;                                                  /* -$1e2a: jump phase, indexes jump_heights[]: 0..3 rising, then down to -1 (a big jump starts at 9) */
extern char climbing_blk48;                                             /* -$1e25: 1 while climbing a column of block 48 (up action 12, down 13) */
extern char climbing_blk25;                                             /* -$1e24: 1 while climbing a column of block 25 (up action 11, down 14) */
extern long zelda_y;                                                     /* -$1e36 */
extern long zelda_screen_y;                                              /* -$1e3e: Zelda's y on the playfield screen (zelda_y - pf_y), in cells */
extern long zelda_sprite_y;                                              /* -$2074: Zelda's sprite draw y (zelda_screen_y - 14); = sprite_draw_ys[0] */
extern long max_hearts;                                                 /* -$23e8 */
extern char cheats;                                                     /* -$1ec2 */
void color_font_draw_glyphs(long x, long y, char *text, long shadow);   /* assembly, gap_01A90+$ae */
long demo_playfield(long room, long track_no, long load_enemies, long unused, long zelda_start_x, long zelda_start_y, long x, long y, unsigned char resume, unsigned char last_room);
void init_projectile(long type, long x, long y, long vx, long vy, long accel_y, long num_repeats, long start_delay, long fire_interval, long life_frames, long hit_damage);
void kill_sprite(long sprite_no);

struct sprite_palette {             /* pointed to by -$240c */
    char pad[0x188];
    long cycle2[32];                /* +$188 */
};
extern long lct_plane_a1;                       /* -$2260: plane A line control table showing plane_a_buf0 */
extern long lct_plane_a2;                       /* -$2264: plane A line control table showing plane_a_buf1 */
extern long lct_plane_b1;                       /* -$2268: plane B line control table (normally plane_b_buf0) */
extern long lct_plane_b2;                       /* -$226c: second plane B line control table */
extern char menu_scrolling;                     /* -$1882 */
extern short sword_level2;                      /* -$2bac */
extern short sword_level3;                      /* -$2bae */
extern long cycle_colors[];                     /* -$2626 */
extern struct sprite_palette *sprite_palette;   /* -$244e */
void set_icf(long plane, long which, long value);
void scroll_position(long y);
void erase_item_name(char slot, long color);
void erase_selected_icon(long where);
void icon_to_from_save_buffer(char slot, long where, long to);

extern long play_done;                  /* -$2c0a: set by the signal handler when a play (load) has finished */
extern char music_paused;               /* -$23d6 */
extern long audio_path;                 /* -$2254 */
extern long death_y;                    /* -$3808 */
extern long saved_zelda_y;               /* -$211c */
extern long saved_zelda_x;               /* -$20c8 */
extern long deaths;                     /* -$37f8 */
extern long deaths_shown;               /* -$37fc */
long check_experience(long exp_no);
void erase_death_number(long y);
void restart_music(void);

struct imagery {                    /* tool imagery file: offsets of 24 compiled sprites */
    long pad[3];
    long off[24];                   /* +$0c, from the start of the code at +$9c */
};
struct alt_action {                 /* 6 bytes, table at -$1c12 */
    unsigned char id;
    char pad;
    short *frames;                  /* +$02 */
};
struct macro {                      /* pointed to by macros[] */
    long state;                     /* +$00 */
    unsigned short enemy_id;        /* +$04 */
    char pad0[9];
    char moved;                     /* +$0f */
    char animate;                   /* +$10 */
    char pad1[5];
    char kind;                      /* +$16 */
    char layer;                     /* +$17 */
    char step_delay;                /* +$18 */
    char step_timer;                /* +$19 */
    char alt_active;                /* +$1a */
    char alt_step_delay;            /* +$1b */
    short sleep_timer;              /* +$1c */
    char wake_pending;              /* +$1e */
    char in_wake_box;               /* +$1f */
    unsigned char lives;            /* +$20 */
    char respawned;                 /* +$21 */
    unsigned short lo;              /* +$22 */
    unsigned short hi;              /* +$24 */
    short target;                   /* +$26 */
    short spread;                   /* +$28 */
};
struct file_pos {                   /* an open NVRAM file (nvr_open_into opens one, nvr_create creates one) */
    long path;                      /* +$00 */
    short color;                    /* +$04: memory colour of the struct (always $81) */
    unsigned short sum;             /* +$06: header, 8 bytes on disc: checksum (low 14 bits) */
    unsigned short host_off;        /* +$08: offset of the packed host string (8, the header size) */
    unsigned short info_off;        /* +$0a: offset of the packed info string */
    unsigned short data_off;        /* +$0c: offset of the game data */
    long pos;                       /* +$0e */
};
extern char file_name[];                        /* -$18fc */
extern char host_name[];                        /* -$190e */
extern struct alt_action sprite_actions[][2];   /* -$1c4e */
extern char hero_action_of[];                   /* -$1cbe: hero_action as an array */
extern struct macro *macros[];                  /* -$26d6 */
extern long lct_head_a;                         /* -$2270: 4-line plane A head LCT; its line 2 links to lct_plane_a1 or a2 */
extern long anim_flip;                          /* -$1926 */
extern long plane_a_buf0;                       /* -$2228: plane A display buffer 0 (pixels from +32), shown by lct_plane_a1 */
extern long zelda_dir;                           /* -$1e32 */
extern char mkt_demo_end;                       /* -$3776 */
extern unsigned char *mkt_demo_ptr;             /* -$3774 */
extern unsigned char mkt_demo_byte;             /* -$1e64 */

extern short help_save[];               /* -$15c4: items saved while the help demo plays */
extern char help_on;
extern unsigned char initial_counts[];
extern long item_icons[];
extern char rope_out;
extern long rope_x;
extern long rope_bottom;
extern long rope_top;
extern long lantern_size;
extern long lantern_pos;
extern char locked_text[];
extern long errno;                      /* -$aa4: error code of the file routines */
void draw_selected_icon(char slot, long where);
void draw_highlighted_icon(char slot, long where);
void draw_item_name(char slot, long color);
void font_draw(long x, long y, char *text, long z);

struct pcb {                        /* CD-i play control block (PCB) of ss_play; pointed to by -$2b9e */
    short stat;                     /* +$00 */
    short sig;                      /* +$02 */
    long rec;                       /* +$04 */
    long chan;                      /* +$08 */
    short achan;                    /* +$0c */
    char *video;                    /* +$0e */
    char *audio;                    /* +$12 */
    long data;                      /* +$16 */
};
extern struct pcb *pcb;
extern long plane_a_buf1;               /* plane A display buffer 1, shown by lct_plane_a2; also scratch space for loads */
extern long voice_done;
extern long *sprite_frames;
extern long cur_sprite;
extern char big_jump_active;            /* 1 during a big jump (item 3, the Magic Cape): move_link uses big_jump_up/down */
extern long zelda_idle_timer;            /* frames with no action; at 5 Zelda returns to the stand pose */
extern char attack_state[];             /* per-sprite combat state 0-4 */
extern long sprite_damage[];
extern char attack_damage[];            /* nonzero while attacking: the damage dealt */
extern long knockback[];                /* signed knockback distance (the sign is the direction) */
extern char freeze_timer;               /* frames left in which combat-type-4 enemies are frozen (item 4, the Flute) */
extern long plane_a_data;               /* 0x16800-byte area after plane A's two buffers (tables, staging, graphics) */
extern long plane_b_buf1;               /* plane B buffer 1 (+0x16800) */
extern long plane_b_buf2;               /* plane B buffer 2 (+0x2d000): animation backgrounds */
extern long glove_flash;                /* power glove blast countdown */
extern long lantern_level;
extern unsigned short music_chan;
extern long play_failed;                /* set by the signal handler on a play error; play_error() then retries */
void plane_B_over_A(void);
void flip_A(void);
void display_clean_disc_sign(long n);
void fade_up_plane_A(void);
void plane_A_over_B(void);
void attenuation(long n);
void my_sprintf(char *buf, long digits, long value);
void color_font_draw(long x, long y, char *text, long dest_buf, long color);
void init_enemy_projectiles(void);
void start_voice_over(long voice_no);
void set_pcb(short stat, short sig, long rec, long chan, short achan, char *video, char *audio, long data);

long play_sound(long sound_id);
void stopsound(long sound_id);
void init_bomb_hit(long x, long y, long direct_hit);

extern long macro_ptrs[];
extern char *macro_data;
extern long lantern_level2;
extern char rubies_text[];
extern char rubies_num[];
extern long sprite_xs[];
extern long sprite_ys[];

long nvr_free_mem(long ret_size, char *ret_mem);
char *nvr_alloc_mem(long mem_size, long mem_color);
long nvr_close(struct file_pos *nvr_file);
long nvr_check_name(char *name);
long nvr_checksum(struct file_pos *fp);

extern char nvr_magic[];                /* -$42 */
extern char nvr_key[];                  /* -$3e */
long nvr_unpack_name(char *in, char *out, long n, char *key, long keylen);
long nvr_read_host(struct file_pos *fpos, char *out);
struct file_pos *nvr_open_into(char *name, char *want, long mode, struct file_pos *file_rec);

long nvr_pack_name(char *in, char *out, long n, char *key, long keylen);

extern char clean_text1[];
extern char clean_text2[];
extern long line_step;
extern long line_count;
extern long thrust_lunged;              /* 1 if this thrust moved Zelda forward 4 cells (undone next frame) */
extern char mem_space[];                /* -$18ea */
void end_thrust(void);

struct block_info {                 /* filled by get_sprite_block */
    long left;                      /* +$00 */
    long right;                     /* +$04 */
    long climb_a;                   /* +$08 */
    long climb_b;                   /* +$0c */
    char step;                      /* +$10 */
    char solid;                     /* +$11 */
    char f12;                       /* +$12 */
    char f13;                       /* +$13 */
    char f14;                       /* +$14 */
    char f15;                       /* +$15 */
    char door;                      /* +$16 */
    char deadly;                    /* +$17 */
    char ground;                    /* +$18 */
    char f19;                       /* +$19 */
    char floor;                     /* +$1a */
    char f1b;                       /* +$1b */
    char f1c;                       /* +$1c */
    char f1d;                       /* +$1d */
    char track;                     /* +$1e */
    char pad4;
};
extern char death_done;
extern char error_bufs[];               /* 30-byte PCL error records (init_error_buffer) */
extern long sprite_drawn[];             /* sprite was drawn last frame (its old spot needs erasing) */
extern long sprite_asleep[];            /* enemy is dormant, waiting to wake */
extern long sprite_draw_xs[];           /* sprite draw x in cells (map x - 8 or 9) */
extern long sprite_draw_ys[];           /* sprite draw y in cells (map y - 14) */
extern char help_ended;                 /* the help demo was just left */
void get_sprite_block(long x, long y, struct block_info *info);

struct channel {                    /* 22 bytes, array at -$1286 */
    long next;                      /* +$00 */
    long f4;                        /* +$04 */
    char pad0[4];
    unsigned char flags;            /* +$0c */
    char pad1;
    unsigned long len;              /* +$0e */
    long data;                      /* +$12 */
};
extern char dark_room;                      /* the room is dark (in dark_rooms) */
extern unsigned short *alt_macro_lists[];   /* per-sprite alternate macro script */
extern unsigned char alt_frame_base[];      /* image offset of alternate-macro images: 0 facing right, 3 facing left */
extern char mm_menu_shown;                  /* plane A menu fully scrolled down */
extern short music_flags;
extern long music_path2;
extern char cursor_data[];
extern char alt_repeat[];               /* -$1b38 */
extern unsigned char alt_index[];       /* -$1b22: position in alt_macro_lists[n] (-1 at the start) */
extern struct channel channels[];       /* -$1286 */
void reinit_lct_B(void);
void dim_lantern(void);
void get_new_macro_image(long sprite_index, unsigned char flip, unsigned short code);
void reinit_sprite(long sprite_idx);

struct stick {
    long x;                         /* +$00 */
    long y;                         /* +$04 */
    long z;                         /* +$08 */
    long count;                     /* +$0c */
};
struct label {                      /* 18 bytes, array at -$3578: a place name on the world map */
    unsigned short x;               /* +$00 */
    unsigned short y;               /* +$02 */
    unsigned short w;               /* +$04 */
    unsigned short h;               /* +$06 */
    unsigned short hot_x;           /* +$08: the hotspot (map coordinates) that shows the label */
    unsigned short hot_y;           /* +$0a */
    unsigned short hot_w;           /* +$0c */
    unsigned short hot_h;           /* +$0e */
    short group;                    /* +$10: labels of one place share a group */
};
extern char mod_dir_a[];
extern char mod_dir_b[];
extern long cmaps[];
extern long zelda_cmap;                  /* cmaps[0]: Zelda's collision maps */
extern long enemy_cmaps;                /* cmaps[1..3] */
extern long proj_cmap_base;             /* cmaps[4..5]; proj_cmaps[] point into it */
extern char death_text[];
extern struct label labels[];
extern long label_save_buf;             /* map pixels saved under the shown label */
void fill_tool_list(void);

extern long boss_flag;
extern char jump_falling;               /* 1 during the falling phase of a jump */
extern struct block_info jump_info;
void decode_mem_space(void);
void check_map_while_jump_descent(long dir);
long nvr_close_free(struct file_pos *file_rec);
struct file_pos *nvr_open(char *name, char *want, long mode);
long nvr_read(struct file_pos *fpos, char *buf, long nbytes);

extern long lct_head_b;                 /* -$2274: 4-line plane B head LCT; its line 2 links to lct_plane_b1 or b2 */
extern long font_glyph_data;            /* -$1126 */
extern char menu_icons_ready;           /* -$2810 */
extern char font_ready;                 /* -$1a0c */
void add_fake_bombs(void);
void add_fake_ropes(void);
void add_fake_oil(void);
void load_font(long data);

extern long music_track;                /* -$228c */
extern struct pcb music_pcb;            /* -$11ca */
extern long room_number;                /* -$2290 */
extern unsigned char dark_rooms[];      /* -$37a4 */
extern struct channel *cur_channel;     /* -$115c */
extern long channel_queue;              /* -$1158 */
extern long mm_game_title;              /* -$16d4: the game's title, for the save files */
extern long mm_banner;                  /* -$1866: banner text drawn at the top of the menu */
extern long mm_no_memory_title;         /* -$181a: the banner when the save memory is full */
extern char mm_banner_color;            /* colour of the banner text */
extern char mm_title_color;             /* colour of the menu title */
extern unsigned char mm_text_color;     /* colour of normal rows */
extern unsigned char mm_hilite_color;   /* colour of the highlighted row */
extern char mm_dim_color;               /* colour of unavailable save slots */
extern char mm_continue;                /* -$187e */
extern unsigned char mm_choice;         /* -$1874 */
extern char mm_done;                    /* leave mmenu_loop */
extern char mm_saved;                   /* a game was saved during this menu visit */
extern long mm_idle_frames;             /* idle frames on the title menu; the attract demo starts at 1400 */
char *init_error_buffer(long buf_index);
long randomize(void);    /* no return statement: returns what is left in d0 */
void debounce_trig(void);
void debounce_stick(void);
void new_mmenu(unsigned char menu_no);
void mmenu_initial_clear(void);
void mmenu_initial_draw(void);
void mmenu_loop(void);
void mmenu_scroll_up(void);

extern char mm_quit_after_save;         /* quit after the save from the "save before quitting?" menu */
extern char mm_quit;                    /* Quit was chosen */
extern char mm_help;                    /* Help was chosen */
extern char mm_new_game;                /* start a new game (also set by the attract demo) */
extern unsigned char mm_cursor;         /* highlighted row */
extern unsigned char mm_old_cursor;     /* row highlighted before */
extern unsigned char mm_row_count;      /* rows in the current menu */
extern char help_aborted;               /* a help or attract demo was cut short: the next menu opens on Help */
extern long old_zelda_dir;               /* -$19ea */
extern unsigned char crouch_wait_timer; /* -$1e26: frames since the last stand-up; crouching needs >= 10 */
extern char zelda_crouching;             /* -$1e27: 1 while Zelda crouches */
extern long pf_plane_b_buffer;          /* -$2288: the same toggle for playfield LCT updates */
struct macro_def {                  /* second argument of init_type_<n> */
    char type;                      /* +$00: which init_type_<n> */
    char pad0[3];
    short x;                        /* +$04 */
    short y;                        /* +$06 */
    char pad1[6];
    short start_state;              /* +$0e */
    short lo;                       /* +$10 */
    short hi;                       /* +$12 */
    short spread;                   /* +$14 */
    char alt_macro;                 /* +$16 */
    char alt_repeat;                /* +$17 */
    char alt_chance;                /* +$18 */
    char alt_dir;                   /* +$19 */
    char alt_speed;                 /* +$1a */
};
struct motion {                     /* argument of check_which_downward_motion (menu_event, -$1da6) */
    long dir;
    long next;                      /* +$04 */
    long prev;                      /* +$08 */
    long fc;                        /* +$0c */
};
struct item_name {                  /* 8 bytes, table at -$1050 */
    char *line1;
    char *line2;
};
void mmenu_scroll_up_intermediate(void);
void mmenu_scroll_down_intermediate(void);
void mmenu_scroll_down(void);
void mmenu_clear_area(void);
void mmenu_redraw(void);
void init_1_or_4(long sprite_idx);
void check_for_climb(long n);
void set_link_position(long dx);
void set_link_position_and_dir(long dx);

unsigned char mm_rows[8];

extern long fall_timer;                 /* -$19e6: frames spent falling; at 3 Zelda takes the fall pose */
extern char jump_info_ground;           /* -$1dea: jump_info.ground (+$18) under another name */
extern long save_size;                  /* -$1890 */
extern char save_host_info[];           /* -$1914 */
extern char save_flags[];               /* -$262e */
extern long room_gfx;                   /* -$22c8: loaded room graphics: label images on the world map, enemy graphics in a room */
extern long rtr_index;                  /* -$2c1a */
extern char data_file_name0[];          /* -$16bc */
extern char data_file_name1[];          /* -$16b2 */
extern char data_file_name2[];          /* -$16a6 */
void create_file_and_host_names(long slot_number);
long nvr_avail(void);
long nvr_delete(char *name);
struct file_pos *nvr_create(char *name, char *str_a, char *str_b, long extra, long mode);
long nvr_write(struct file_pos *fp, char *buf, long write_len);
void encode_mem_space(void);
void open_up_area(long area, long flag);
void clean_disc(void);
void do_mmenu_code(void);
void quit_game(long n);

extern long *lct_a;                     /* -$2294 */
extern long bg_cycle[];                 /* -$25a6 */
void font_erase(long x, long y, char *text, long dest_buf);
void draw_heart(short kind, short heart_no, long y);

extern char zelda_riding;                /* -$1e1f: 1 while Zelda stands on a moving type-6 platform */
extern long music_wait;                 /* -$11ac */
extern short music_side;                /* -$115e */
extern char sig0_flags;                 /* -$1202: sig_blocks[0].flags; bit 7 = buffer done */
extern char sig1_flags;                 /* -$11e6: sig_blocks[1].flags */
extern long sig0_count;                 /* -$11ea: sig_blocks[0].count */
extern long sig1_count;                 /* -$11ce: sig_blocks[1].count */
long ac_process(long sig);
long check_vulnerablility(long sprite_idx, long weapon);
struct enemy_def {                  /* second argument of wake_on_chance */
    char pad0[0x2c];
    short sleep_time;               /* +$2c */
    char pad1[2];
    short sleep_time2;              /* +$30 */
    char pad2[0x17];
    unsigned char chance;           /* +$49 */
    unsigned char chance2;          /* +$4a */
};
void jump_up_dir(long dx);

extern long *sprite_pos[];              /* -$272a: x2 position block of each sprite (16 bytes) */
extern long sprite_buffer;              /* -$22cc */
extern struct block_info *block_ptrs[]; /* -$2682 */
extern char *mm_start_text;             /* -$1800: "Start Game", row 0 before a game has started */
extern unsigned char mm_text_x;         /* -$187c */
void place_raft(long x, long y, long state);

extern long frame_flag;                 /* -$2736 */
extern long death_x;                    /* -$3804 */
long text_width(char *text);
extern long enemy_data;                 /* -$22ac: 176 bytes per enemy */
void start_continue(void);
void update_death_pose(void);
void draw_death_number(long y);
void death_loop_bottom(void);
long old_get_sprite_block(long x, long y);

struct smap {
    char pad0[10];
    long data;                      /* +$0a */
    long size;                      /* +$0e */
    char pad1[8];
    short w;                        /* +$1a */
    short pos;                      /* +$1c */
};
struct sound_pcb {                  /* 4 bytes */
    short a;
    short b;
};
extern long audio_signal;               /* -$1152 */
extern long audio_task;                 /* -$114e */
extern char music_file_name[];          /* -$12a8 */
extern short sound_level;               /* -$1150 */
void load_effects(void);
void add_to_tool_list(char tool);
void check_lamp_highlight(char item);
void check_canteen_highlight(char item);

extern char slot_used[];                /* -$262a */
extern long sprite_frame_no[];          /* -$1d66 */
extern long pf_x_px_max;                /* -$1544 */
extern long pf_y_px_max;                /* -$1548 */
void explode_bomb(unsigned char tool_idx);
void add_bombs_to_buy(void);
void add_ropes_to_buy(void);
void add_oil_to_buy(void);
void delete_tool(unsigned char tool_index);
void clear_area_experiences(void);
void init_playfield_size(long room);
long drop_sprite(long track, long enemies, long zelda_x0, long zelda_y0, unsigned char resume, unsigned char last);
void stop_music(void);

extern unsigned short bit_masks[];      /* -$248e: 1 << n */
extern unsigned char sound_len[];       /* -$128c */
extern char *demo_ptr;                  /* -$3712 */
extern char demo_buf[];                 /* -$3714 */
extern char demo_started;               /* -$23e0 */
void draw_border(long where);
void draw_an_item(long where, short icon, long slot, long count);
void draw_sword(long where);
void draw_shield(long where);
long delayed_wait(long frames);
void screen_dimmer(long on, long count, long mode, long sound, long stereo);
void debounce_trig_music(void);

extern char heart_bonus;                /* -$23dc */
extern char anim_restores_life;         /* -$23da: restore Zelda's life after the cutscene */
extern char anim_flag_239c;             /* -$23de */
extern char bomb_bounced;               /* -$1ec6: the lobbed bomb has bounced off the map once */
struct enemy {                      /* 176 bytes, at enemy_data */
    char type;                      /* +$00 */
    char pad0a;
    short id;                       /* +$02 */
    char pad0b[5];
    char layer;                     /* +$09 */
    char pad0c;
    char step_delay;                /* +$0b */
    short present;                  /* +$0c */
    char pad0d[0x1e];
    unsigned short first_sleep;     /* +$2c */
    char first_wake_mode;           /* +$2e */
    char lives;                     /* +$2f */
    unsigned short sleep;           /* +$30 */
    char mode;                      /* +$32 */
    unsigned char kind;             /* +$33 */
    unsigned char experience;       /* +$34 */
    char pad1[0x13];
    unsigned char item;             /* +$48 */
    char pad2[7];
    char state;                     /* +$50 */
    char pad3[0x5f];
};
void leave_item(long kind, long x, long y);
void mark_experience(long exp_id);
void start_anim(long room, long sprite_no, long kind);
long play_animation(long anim_number);
void start_music(long track);
long check_map_row(long start, long y, long count);
void set_new_map(long spot_x, long spot_y, long spot_w, long spot_h);

extern struct pcl *bg_pcl;              /* -$2c06 */
void go_find_action(long who, long id);
void draw_initial_death_screen(long y);
long death_loop(void);
void zero_video_cil(void);
void zero_data_cil(void);
void set_pcl(struct pcl *pcl, char ctrl, char c2, char c3, short sig, char *nxt, long buf, long buf_size,
             long err, long cnt);

extern long pf_extra_screens;           /* -$1540: screens beyond the first in the scroll direction */
extern char pf_scrolls_horiz;           /* -$153c: 1 = the playfield scrolls horizontally (also for one screen), 0 = vertically */
extern long map_height;                 /* -$153a */
extern long map_width_px;               /* -$152a */
extern long map_height_px;              /* -$1536 */
extern char room_widths[];              /* -$34c0 */
extern char room_heights[];             /* -$350c */
extern long bg_loaded;                  /* -$1922 */
extern long frame_count;                /* -$1e4a */
extern struct pcl *pcl_b;               /* -$2c02 */
extern struct pcl *pcl_c;               /* -$2bfe */
void ac_signal_process(long sig);
void init_type_2(long sprite_no, struct macro_def *def);
void init_type_3(long sprite_idx, struct macro_def *def);
void init_type_7(long sprite_index, struct macro_def *def);
void init_type_8(long sprite_no, struct macro_def *def);
void init_type_5(long sprite_idx, struct macro_def *def);
void move_type_5(long sprite_no);
struct macro_def6 {                 /* second argument of init_type_6 */
    char pad0[0xe];
    unsigned char state;            /* +$0e */
    unsigned char kind;             /* +$0f */
    char pad1[0x20];
    short pause_time;               /* +$30 */
};
struct macro_def5 {                 /* second argument of init_type_5; move_type_5 reads it from enemy_data */
    char pad0[0xe];
    short start_state;              /* +$0e: 2 = starts going right */
    short lo;                       /* +$10: x to turn round at going left */
    short hi;                       /* +$12: x to turn round at going right */
    short spread;                   /* +$14 */
    unsigned char kind;             /* +$16: its macro, 0 = no sprite */
    unsigned char alt_kind;         /* +$17: macro it sometimes switches to */
    unsigned char speed;            /* +$18: 1-8 */
    char alt_loops;                 /* +$19: times the alt macro plays */
    unsigned char alt_chance;       /* +$1a: chance of the alt macro (10 = always) */
    char alt_delay;                 /* +$1b: frames between tries, 0 = never */
    unsigned char stop_kind;        /* +$1c: macro on a stop block ($5d) */
    unsigned char speed2;           /* +$1d: 1-8 */
    char stop_loops;                /* +$1e: times the stop macro plays */
};
struct macro5 {                     /* a macro as init_type_5 and move_type_5 see it */
    char pad0[0x22];
    short target_x;                 /* +$22: x where it turns round */
    char spread;                    /* +$24: spread * 10 */
    unsigned char frame_offset;     /* +$25: 0 going right, 3 going left */
    unsigned char kind;             /* +$26: the macro it plays now */
    char alt_timer;                 /* +$27: counts down alt_delay */
    char loops;                     /* +$28: plays left of the alt or stop macro */
};
void init_5_or_6(long sprite_no, long macro_index, long dx, long dy, long size);

extern long map_cursor_x;               /* -$159c: overworld cursor x on screen; the point under it is map_x + map_cursor_x */
extern long map_cursor_y;               /* -$15a0: overworld cursor y on screen */
extern long saved_map_cursor_x;         /* -$15a4: map_cursor_x kept while in an area */
extern long saved_map_cursor_y;         /* -$15a8: map_cursor_y kept while in an area */
extern long saved_map_x;                /* -$15ac: overworld scroll x kept while in an area */
extern long saved_map_y;                /* -$15b0: overworld scroll y kept while in an area */
void main_init(long sprite_no, struct macro_def *def);

extern char *palette_file;              /* -$22e0 */
void init_miss(long x, long y);
void convert_palette(void);
long play_error(void);

extern long is_pal;                     /* -$2280: 1 on a 625-line (PAL) display */
void retrieve_help_state(long *stick_event, long *button_state);
void retrieve_mkt_demo(long *x, long *y);

extern long plane_a_extra;              /* -$2248: second plane A allocation (0x1fc00): macro data, room graphics, palette file, sounds */
extern long plane_b_data;               /* -$2244: rest of plane B (+0x43800): block_map, collision maps, sprite buffers, enemy data */
extern long tool_imagery_buf;           /* the room's tool sprite imagery */
extern long sfx_data;                   /* sample data of sound channels 0-3 and 5 */
extern long ctl_tables;                 /* PCB, CILs, PCLs, then line_table/lct_a/lct_b */
extern long menu_icons_buf;             /* menu and subscreen icon imagery */
extern long proj_gfx_buf;               /* projectile graphics (two enemy_gfx blocks); same address as label_save_buf */
extern long proj_tables;                /* proj_defs, then the projectiles at +0x168 */
extern long zelda_imagery;               /* Zelda's sprite imagery */
extern long sfx4_data;                  /* sample data of sound channel 4 */
extern long env_proj_buf;               /* the room's environment projectile list */
extern long enemy_list;                 /* the room's enemy type ids (ending with -1); enemy records follow at +32 */
extern long path_blocks;                /* per sprite, 18 map blocks saved under riding paths */
extern long path_blocks_r;              /* per sprite, 18 map blocks saved under a type-7 sprite's right column */
extern long frame_color_hi;             /* per sprite and frame: high byte of the erase colour */
extern long frame_color_lo;             /* per sprite and frame: low byte of the erase colour */
void init_cmap_addresses(void);
void init_table_widths(void);

struct tool_spot {                  /* 6 bytes, table at -$da8: where tools lie in the rooms */
    unsigned char kind;
    unsigned char room;             /* +$01 */
    short x;                        /* +$02 */
    short y;                        /* +$04 */
};
long check_if_have_tool(long tool_id);

extern long zelda_target_y;              /* -$1e5a */
extern long zelda_target_x;              /* -$1e56 */
void inc_pf_y(void);
void dec_pf_y(void);
void inc_pf_x(void);
void dec_pf_x(void);
void write_sprite_blocks(long x, long y, long nblocks, char *save, long value);
void restore_sprite_blocks(long x, long y, long count, char *src);
long get_image_number(long sprite_idx);

struct proj_def {                   /* 12 bytes, table at proj_defs */
    long anim_left;
    long anim_right;
    char owner;                     /* +$08 */
    char stops_at_walls;            /* +$09 */
    char bounces;                   /* +$0a */
    char offscreen_ok;              /* +$0b */
};
extern struct proj_def *proj_defs;      /* -$14bc */
struct macro_b {                    /* a macro as move_type_2 sees it */
    char pad0[0x22];
    char macro_no;                  /* +$22 */
    char loops;                     /* +$23 */
    char loops_left;                /* +$24 */
    char dir_mode;                  /* +$25 */
    char frame_offset;              /* +$26 */
    unsigned char alt2_macro;       /* +$27 */
    char alt2_repeat;               /* +$28 */
    char alt2_chance;               /* +$29 */
    char b2a;                       /* +$2a */
    char b2b;                       /* +$2b */
    char alt3_chance;               /* +$2c */
    char alt3_dir;                  /* +$2d */
    char alt3_speed;                /* +$2e */
};
long check_for_alternate_macro(long sprite_idx, long state);

extern char bumper_dir[];               /* -$1598 */
extern char bumper_module[];            /* -$158c */
extern char bumper_dir2[];              /* -$157e */
extern long bumper_proc;                /* -$155c */
extern long bumper_env;                 /* -$a90 */
extern char *bumper_name;               /* -$192a */

extern long *env_projectiles;           /* -$14c4: 14 longs each, the arguments of init_projectile */
extern long proj_min_x;                 /* -$14c8 */
extern long proj_max_x;                 /* -$14cc */
extern long proj_min_y;                 /* -$14d0 */
extern long proj_max_y;                 /* -$14d4 */
extern long last_side;                  /* -$11b0 */
extern short chan_pos;                  /* -$1154 */
void copy_sound(long dest, long src);
extern long sound_mem[100000];
extern long sound_ids[100000];
extern struct sound_pcb sound_pcbs[100000];

extern void (*proj_draw[])();       /* -$13b0: draw routine of each projectile frame */

extern long idle_frames;                /* -$1958: frames without input; the screen dims after a while */

extern long button_pressed;             /* -$2200: button newly pressed this frame: 1 sword, 2 item */
extern long *cil_b;                     /* -$2bea */
extern struct pcl *pcl_d;               /* -$2bfa */
extern struct pcl *pcl_e;               /* -$2bf6 */
extern struct pcl *pcl_f;               /* -$2bf2 */
void sprite_hits_tool(unsigned char tool_idx, char skip);

extern unsigned char *voice_ptr;        /* -$3732 */
extern char voice_ready;                /* -$23d8 */
extern unsigned long repeat_count;      /* -$1062 */
long get_first_item(long refill);
void highlight_item(char item, long where);
void unhighlight_item(char item, long where);
extern long sprite_macros[];            /* -$1ab8 */

struct macro_r {                    /* a macro as get_random_bound sees it */
    char pad0[0x22];
    short lo;                       /* +$22 */
    short hi;                       /* +$24 */
    short target;                   /* +$26 */
    short spread;                   /* +$28 */
};



extern char help_repeat;                /* -$1e62 */
extern unsigned char help_cmd;          /* -$1e60 */
extern char voice_wait;                 /* -$3734 */
extern char help_on;                    /* -$3736 */
void restore_vert_blocks(long x, long y, long block_count, char *src);
void set_help_off(void);

extern long knocked;                    /* -$19ee */
extern short zelda_flag_1df2;            /* -$1e2e */
extern short zelda_frames_1[];           /* -$197c: frame lists of Zelda's actions */
extern short zelda_frames_2[];
extern short zelda_frames_6[];
extern short zelda_frames_7[];
extern short zelda_frames_8[];
extern short zelda_frames_9[];
extern short zelda_frames_10[];
extern short zelda_frames_11[];
extern short zelda_frames_12[];
extern short zelda_frames_13[];
extern short zelda_frames_14[];
extern short zelda_frames_15[];
extern short zelda_frames_16[];

extern char mkt_demo_on;                /* -$377a */
extern unsigned char mkt_demo_buf[];    /* -$3770 */
void mmenu_choice(long selection);
void wait_for_menu_option(void);

struct sig_block {                  /* 28 bytes, two at -$1202 (sig0_flags, sig1_flags) */
    char flags;                     /* +$00 */
    char pad0;
    char a;                         /* +$02 */
    char b;                         /* +$03 */
    short sig;                      /* +$04 */
    char *next;                     /* +$06 */
    long mem;                       /* +$0a */
    long one;                       /* +$0e */
    char *err;                      /* +$12 */
    char pad1[2];
    long count;                     /* +$18 */
};
extern struct sig_block sig_blocks[2];  /* -$1202 */
void write_vert_blocks(long x, long y, long count, char *save, long value);

extern long sprite_gfx[];               /* -$278a: enemy_types index + 1 of each sprite */
extern char help_demo[];                /* -$370e */
extern unsigned char voice_buf[];       /* -$372e */
void check_collisions(long sprite_idx, long cmap_idx, long proj_idx, long proj_frame);
void check_map(long proj_idx, long proj_frame);
void roll_credits(void);
void set_mkt_demo_off(void);
void set_ov_help_off(void);
void mmenu_init(void);



extern long clut_table[];               /* -$2e36: plane A CLUT load commands (install_palette) */
char pick_up_perm_tool(unsigned char tool);
void more_items(long item, long add, long max);

extern struct tool_spot tool_spots[];

struct macro_alt {                  /* a macro's alternate-macro settings */
    char pad0[0x27];
    char alt2_macro;                /* +$27 */
    char alt2_repeat;               /* +$28 */
    char alt2_chance;               /* +$29 */
    char b2a;                       /* +$2a */
    char b2b;                       /* +$2b */
    char alt3_chance;               /* +$2c */
    char alt3_dir;                  /* +$2d */
    char alt3_speed;                /* +$2e */
};
long set_alternate_macro(long sprite_idx, long which, long repeat, long state, long speed);
void do_alternate_macro(long sprite_no);

struct enemy_st {                   /* the part of an enemy record from +$50 */
    char state;                     /* +$50 */
    char pad0[0x1b];
    short count;                    /* +$6c */
};


extern short rope_count;                /* -$2bce */
extern long rope_anim_toggle;           /* -$1a0a: 0/1 per frame while climbing the rope */
long check_for_rope_path(long x, long y);

extern unsigned char ov_help_cmd;       /* -$1626: overworld help script: current command byte */
extern long ov_help_repeat;             /* -$161c: overworld help script: frames left for the current move */
extern long map_scroll_dx;              /* -$1620: overworld scroll speed in x */
extern long map_scroll_dy;              /* -$1624: overworld scroll speed in y */

void slide(long dx, long wall_flag);

struct macro_def2 {                 /* second argument of init_type_2 */
    char pad0[0xe];
    char macro_no;                  /* +$0e */
    char loops;                     /* +$0f */
    char dir_mode;                  /* +$10 */
    char alt_macro;                 /* +$11 */
    char alt_speed;                 /* +$12 */
    char alt_repeat;                /* +$13 */
    char alt_chance;                /* +$14 */
    char alt_dir;                   /* +$15 */
};

extern char str_625[];                  /* -$1978: "625": a 625-line (PAL) display in the video device parameters */
extern char str_tv[];                   /* -$1974: "TV" */
extern char cd_dev_name[];              /* -$1970: the CD device name */
extern long plane_b_mem;                /* -$2234: start of plane B's memory (0x4d800 bytes, srqcmem colour 0x81) */
extern long lct_pal_a;                  /* -$2278: 40-line plane A top border LCT (PAL only) */
extern long lct_pal_b;                  /* -$227c: 40-line plane B top border LCT (PAL only) */
extern long dcp_regs_a[];               /* -$2e76 */
extern long dcp_regs_b[];               /* -$2ea6 */

extern char *mm_titles[];               /* -$1862: each menu's title */

extern long proj_cmaps[];
void init_bomb(long x, long y);

void finish_move(long state, long dx, long action, long step, long sprite_no);
void get_random_bound(long sprite_no, long state);

struct enemy_type {                 /* 8 bytes: an enemy kind and its graphics */
    long id;
    struct enemy_gfx *gfx;          /* +$04 */
};
struct enemy_gfx {
    long f0;
    long f4;                        /* +$04 */
    long f8;                        /* +$08: f4 + f8 is the size of the block (the next one follows) */
    long frames[24];                /* +$0c: offsets from +$9c */
    short colors[24];               /* +$6c */
};
extern struct enemy_type enemy_types[]; /* -$1a40 */
extern long sprite_drawn_frame[];       /* -$1dba: frame drawn last time (picks the erase colours) */

extern long in_area;                    /* -$1934: 1 while Zelda is in an area, 0 on the map */

extern long clut_table_lo[];            /* -$2e32: = &clut_table[1]: plane A colours 0-63 as CLUT load commands */
extern long clut_table_hi[];            /* -$2d2e: = &clut_table[0x42]: plane A colours 64-127 */

extern short bg_cycle_start[];          /* -$2522: per colour-cycle range (6): first bg_cycle entry, 1-based */
extern short bg_cycle_len[];            /* -$24fe: per range: number of colours (cycles from 2) */
extern short bg_cycle_delay[];          /* -$2516: per range: frames between steps (0 = off) */
void reset_pcl(struct pcl *pcl, char ctrl, char *buf, long cnt);

struct climb_dir {                  /* climb's last argument */
    long dir;
    long next;                      /* +$04 */
    long prev;                      /* +$08 */
};
extern long climb_toggle;               /* -$19fa */
void jump_off(void);

extern unsigned short low_bits_mask[];  /* -$24ae: [n] = bits 0..n set */
extern unsigned short high_bits_mask[]; /* -$24ce: [n] = bits n..15 set */
void init_hit(long x, long y);
void hero_hit(long damage);
void reflect_projectile(long proj_no);

extern short world_cycle_start[];       /* -$2546: bg_cycle_start on the world map */
extern short world_cycle_len[];         /* -$252e: bg_cycle_len on the world map */
extern short world_cycle_delay[];       /* -$253a: bg_cycle_delay on the world map */
extern short bg_cycle_timer[];          /* -$250a: per range: countdown to the next step */
extern char bg_cycles_on;               /* -$24f0: cycle the background colours every frame */
void read_bg_cycle_colors(void);
void check_world_hotspots(void);

extern long anim_delay;                 /* -$280e: frames to wait before the pending cutscene */
void kill_sprite_noflash(long sprite_idx);

void get_collision_result(long high_bit, long low_bit, unsigned short *mask, long unused, long down, long flip, long *hit, long *pos);

void check_which_downward_motion(struct motion *motion_ptr);
void check_which_upward_motion(struct motion *move_info);
void fix_jump_stick(struct stick *stick_ev);
void jump_up(struct motion *move_info);
void handle_knockback(void);
void using_rope(void);
void big_jump_up(void);
void big_jump_down(void);
void sword_thrust(void);
void climb(unsigned char up_action, unsigned char climb_action, long ladder_flag, struct climb_dir *dir_state);
void check_map_while_walking(long dir);

void load_backgrounds(long n);
void flip_backgrounds(long pos);
extern long bg_frames_b[];              /* -$1666 */
extern long palette_in_lo[];            /* -$2b98: = &palette_in[1]: plane B colours 0-63 */
extern long palette_in_hi[];            /* -$2a94: = &palette_in[0x42]: plane B colours 64-127 */

long check_buttons(void);

extern char proj_frames_0[];            /* -$14d6 */
extern char proj_frames_1[];            /* -$14d8 */
extern char proj_frames_6[];            /* -$14da */
extern char proj_frames_2[];            /* -$14dc */
extern char proj_frames_3[];            /* -$14de */
extern char proj_frames_5[];            /* -$14e0 */
extern char proj_frames_7[];            /* -$14e2 */
extern char proj_frames_8[];            /* -$14e4 */
extern char proj_frames_9[];            /* -$14e6 */
extern char proj_frames_10[];           /* -$14e8 */
extern char proj_frames_11[];           /* -$14ea */
extern char proj_frames_12[];           /* -$14ec */
extern char proj_frames_13[];           /* -$14ee */
extern char proj_frames_14[];           /* -$14f0 */
extern char proj_frames_15[];           /* -$14f2 */
extern char proj_frames_16[];           /* -$14f4 */
extern char proj_frames_17[];           /* -$14f6 */
extern char proj_frames_18[];           /* -$14f8 */
extern char proj_frames_19[];           /* -$14fa */
extern char proj_frames_20[];           /* -$14fc */
extern char proj_frames_21[];           /* -$14fe */
extern char proj_frames_22[];           /* -$1500 */
extern char proj_frames_23[];           /* -$1502 */
extern char enemy1_proj_right[];        /* -$150e */
extern char enemy1_proj_left[];         /* -$1512 */
extern char enemy2_proj_right[];        /* -$1516 */
extern char enemy2_proj_left[];         /* -$151a */
extern char enemy3_proj_right[];        /* -$151e */
extern char enemy3_proj_left[];         /* -$1522 */

extern long area_deaths;                /* -$191e: deaths since entering this area (a continue needs < 3) */

extern short death_frames[];            /* -$21fc: the frames of a dying sprite, 999 at the end */
extern char sprite_cycles_on;           /* -$24f2 */
extern long frame_alarm;                /* -$2732: id of the alarm that sends the frame signal $1f9 (alm_cycle) */
extern long frame_count_b;              /* -$1e4e */
extern long button_held;                /* -$1e52: frames a button has been held */
extern long combat_frames;              /* -$1e5e */
extern long combat_frame;               /* -$2806 */
extern long old_pf_x_px;                /* -$24de */
extern long old_pf_y_px;                /* -$24e2 */

long get_abs_x(long which, long x, long y, long dir);
void set_walker(long sprite_idx, long x);
void end_alternate_macro(long sprite_no);
void check_for_restart(long sprite_no);

/* Library routines: the OS-9 / CD-i C library linked into the module's tail (segment 9), called through
   the jump table (the A5 offset of each entry is in config/symbols.txt). They are named after the library
   call each one makes (the system call, and for the CD-RTOS drivers the setstat/getstat function code);
   the comment says which. Names marked (?) are a best guess from how the game uses the routine.
   Their types are what the game's calls need, not the library's real prototypes. */

/* C library */
long strlen(char *s);
long strcmp(char *a, char *b);
void strcpy(long *dest, char *src);
long findstr(long pos, char *string, char *pattern);       /* position of pattern in string, or 0 */
void memset(long buf, long value, long n);
void memmove(long dest, long src, long n);
void free(char *p);

/* OS-9 system calls */
long open(char *name, long mode);                                                         /* I$Open */
long close(long path);                                                                    /* I$Close */
long create(char *name, long mode, long attr, long size);                                 /* I$Create */
long unlink(char *name);                                                                  /* I$Delete */
long read(long path, char *buf, long n);                                                  /* I$Read */
long write(long path, char *buf, long n);                                                 /* I$Write */
long lseek(long path, long pos, long mode);                                               /* I$Seek */
void exit(long code);                                                                     /* F$Exit */
long getpid(void);                                                                        /* F$ID */
void kill(long pid, long sig);                                                            /* F$Send: send a signal */
void wait(long *status);                                                                  /* F$Wait */
void sigmask(long level);                                                                 /* F$SigMask: 1 holds signals, -1 releases */
void sysdate(long format, long *time, long *date, short *day, long *tick);                /* F$Time */
char *srqcmem(long size, long color);                                                     /* F$SRqCMem: memory of a colour ($80 plane A, $81 plane B) */
long srtmem(long size, char *p);                                                          /* F$SRtMem: return memory */
char *modcload(char *name, long mode, long color);                                        /* F$Load into memory of a colour */
char *modlink(char *name, long mode);                                                     /* F$Zelda */
long munlink(char *module);                                                               /* F$UnLink */
long os9exec(char *start, long name, long *argv, long envp, long a, long b, long paths);  /* runs a module (start: os9fork) */
long os9fork(void);                                                                       /* F$Fork (passed to os9exec, not called) */
long alm_cycle(long sig, long time);                                                      /* F$Alarm A$Cycle: send sig every time; returns the alarm id */
void alm_delete(long id);                                                                 /* F$Alarm A$Delete */
char *csd_devname(long type, long n);                                                     /* name of the n-th device of a type (CSD) */
char *csd_devparam(char *name);                                                           /* the device's parameter string (CSD) */

/* CD-RTOS: CD playback (I$SetStt on the CD path) */
void ss_play(long path, struct pcb *p);                     /* SS_Play ($2d): play records into the PCB's buffers */
void ss_abort(long path);                                   /* SS_Abort ($31) */
void cd_setstat_39(long path);                              /* setstat $39 on the raw CD device (?) */

/* CD-RTOS: audio (I$SetStt/I$GetStt on the audio path) */
long sm_creat(long path, long type, long n, long *mem);     /* SS_SM ($3b) function 0: create a sound map */
long sm_out(long path, long id, struct sound_pcb *p);       /* SS_SM 1: play a sound map */
void sm_off(long path);                                     /* SS_SM 2: stop */
struct smap *sm_info(long path, long id);                   /* getstat SS_SM 0: the sound map's descriptor */
void sc_atn(long path, long value);                         /* SS_SC ($3d) 0: attenuation */

/* CD-RTOS: video (I$SetStt SS_DC ($56) on the video path; the function code follows the name) */
long dc_crfct(long path, long plane, long length, long attr);                     /* 0: create a field control table */
long dc_crlct(long path, long plane, long lines, long attr);                      /* 6: create a line control table */
void dc_wrfct(long path, long fct, long start, long n, long *table);              /* 2: write FCT instructions */
void dc_wrfi(long path, long fct, long index, long instr);                        /* 4: write one FCT instruction */
void dc_rdlct(long path, long lct, long a, long b, long c, long d, long *out);    /* 7: read LCT instructions */
void dc_wrlct(long path, long lct, long a, long b, long n, long c, long *table);  /* $1c: write LCT instructions */
void dc_wrli(long path, long lct, long line, long column, long instr);            /* $a: write one LCT instruction */
void dc_flnk(long path, long fct, long lct, long line);                           /* $c: link an FCT to an LCT */
void dc_llnk(long path, long lct, long line, long lct2, long line2);              /* $d: link an LCT line to another LCT */
void dc_exec(long path, long fct_a, long fct_b);                                  /* $e: show the two planes' FCTs */
void dc_intl(long path, long on);                                                 /* $f: interlace (?) */
void dc_ssig(long path, long sig, long line);                                     /* $11: send a signal at a line */
void dc_set_display(long path, long mode);                                        /* $13: 0 on a TV or 625-line set, else 1 (?) */

/* CD-RTOS: pointer and cursor (SS_PT ($59), SS_GC ($52)) */
void pt_coord(long path, long *buttons, long *x, long *y);                      /* getstat SS_PT: buttons and position */
void pt_pos(long path, long x, long y);                                         /* SS_PT 3: set the position */
void pt_org(long path, long x, long y);                                         /* SS_PT 4: set the origin */
void gc_pos(long path, long x, long y);                                         /* SS_GC 0: cursor position */
void gc_show(long path);                                                        /* SS_GC 1 */
void gc_hide(long path);                                                        /* SS_GC 2 */
void gc_def(long path, long a, long b, long w, long h, long c, char *data);     /* SS_GC 3: cursor image */
void gc_col(long path, long color);                                             /* SS_GC 4 */
void gc_blnk(long path, long n);                                                /* SS_GC 5: blink */
void gc_org(long path, long x, long y);                                         /* SS_GC 6: hot spot */

/* Assembly routines (gap_00052, $52-$7f8) */
void queue_insert_channel(long *queue, struct channel *c);  /* insert c, sorted by c->f4 */
struct channel *queue_pop_channel(long *queue);             /* remove and return the head, or 0 */
void clear_384x240(long buf);
void clear_416x240(long buf);
void draw_label_save_under(long dest, long src, long buf, long w, long h);  /* copy a label image into the map, saving what it covers */
void restore_under_label(long dst, long buf, long w, long h);
void draw_rle_sprite(long y, long a, long b);                   /* (dest, sprite, stride): RLE sprite cur_sprite[n] at its draw position */
void draw_rle_sprite_flipped(long y, long a, long b);           /* the same, mirrored */
void draw_rle_sprite_flash(long y, long a, long b);             /* the same in cycling colours (Zelda hit) */
void draw_rle_sprite_flash_flipped(long y, long a, long b);
void erase_32x32(long x, long y, long where);
void draw_rle_icon(long where, long a, long b, long image);     /* 72 x 56 RLE image at (x, y) */
void erase_sprite_rows(long x, long y, long where, long a, long b);
void erase_16x16(long x, long w, long y);                       /* (x, y, dest) */
void erase_16x4(long x, long h, long y);                        /* (x, y, dest) */
void draw_line(long x1, long y1, long x2, long y2, long color); /* into draw_target */
long getstat_free_space(long path);                             /* I$GetStt $43: free bytes */
void rotate_bg_cycle(void);                                     /* step the background colour cycles (bg_cycle) */
void rotate_sprite_cycle(void);                                 /* step the sprite colour cycles (cycle_colors) */


extern char bread_timer;                /* -$1ecc: frames left for the bread Zelda put down ($f0 when put down) */
extern short bread_x;                   /* -$1ece: the bread's x in cells */
extern short bread_y;                   /* -$1ed0: the bread's y in cells */
extern char anim_sequence[];            /* -$3794: the 22 animations play_animations shows, in order */
void init_bread(long x, long y);
void blow_out_lantern(void);
void play_animations(void);
void play_all_animations(void);
void restore_arkidai_path(long sprite_no);
void fade_out_plane_B(void);

#endif
