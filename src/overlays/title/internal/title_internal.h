#ifndef LOM_TITLE_INTERNAL_H
#define LOM_TITLE_INTERNAL_H

#include "overlays/title/title.h"
#include "main/cd_resources.h"
#include "main/display.h"
#include "common/gpu_packet.h"
#include "common/tim.h"
#include "main/main.h"
#include "main/audio/akao.h"
#include "common/overlay_audio.h"
#include "common/pad.h"
#include "common/pad_repeat.h"
#include "common/screen_fade_words.h"
#include "main/controller_internal.h"
#include <libgte.h>
#include <libgpu.h>
#include "main/scene_state.h"
#include "common/saved_game.h"

/** @brief Shift applied to the second rand() value when two are combined into the game id. */
#define TITLE_RNG_HIGH_SHIFT 15

/** @brief Display environments and GPU command buffers used by TITLE. */
struct TitleMenuContext
{
    ScreenFrame screen;
    /** @brief Same gap as CheckPSFrame::reserved_tail; TITLE never reads or writes it. */
    char unused_80bc[0x3C10];

    char second_buffer_header[0x40];
    u_long otag_buffer2[FRAME_ORDERING_TABLE_LENGTH];
    DISPENV disp_env2;
    DRAWENV draw_env2;
    RECT back_screen;
};

extern s32 g_playtime_vsync_origin;
extern u8 g_title_selected_item;
extern s32 g_title_menu_exit_state;
extern u8* g_title_audio_bank_base;
extern s32 g_title_idle_countdown;

extern u8 g_title_menu_item_flags[];
extern u8 g_title_visible_item_rank;
extern u8 g_title_anim_frame;
/** @brief Cursor animation frames, as texture U offsets. */
extern u8 g_cursor_blink_u_offsets[];

/** @brief Entry count, then the byte offset of each menu TIM from the table start. */
extern u32 g_title_menu_tim_table[3];
extern s32 g_slot_slide_x;
extern s32 g_slot_slide_y;
extern s32 g_slot_selected_index;
extern s32 g_slot_slide_frames;
/** @brief Weapon-list scroll in pixels; also the V offset of the icon and name columns. */
extern s32 g_weapon_list_scroll;
extern s32 g_weapon_list_scroll_target;
extern s32 g_slot_slide_x_lerped;
extern s32 g_slot_slide_y_lerped;
extern s32 g_weapon_list_scroll_frames;
/** @brief Starting weapon of each weapon category; the chosen one becomes the hero's weapon. */
extern FieldItemRecord g_starting_weapon_records[FIELD_WEAPON_CATEGORY_COUNT];
/* SaveLayoutEntry::flags; the blend mode is the 2-bit field above SEMI_TRANS. */
#define SAVE_LAYOUT_FLAG_FOLLOWS_SLIDE 1
#define SAVE_LAYOUT_FLAG_SEMI_TRANS 2
#define SAVE_LAYOUT_FLAG_BLEND_SHIFT 2

/** @brief One primitive of the new-game picker screen. */
typedef struct
{
    u32 flags : 4; /**< SAVE_LAYOUT_FLAG_* bits. */
    u32 : 4;
    u8 type;
    u8 tex_slot;
    u8 pad;
    s16 x;
    s16 y;
    s16 tile_x;
    s16 tile_y;
    u16 u0;
    u16 v0;
    u16 width;
    u16 height;
    u32 reserved;
} SaveLayoutEntry;

/** @brief Index of each entry in g_save_layout_table. */
enum
{
    SAVE_LAYOUT_CAPTION = 0, /**< Its UV picks the hero caption or the weapon caption. */
    SAVE_LAYOUT_BANNER = 1,

    /* Hero 0's weapon list. The icon and name columns scroll through their V. */
    SAVE_LAYOUT_HERO0_ICONS = 2,
    SAVE_LAYOUT_HERO0_NAMES = 3,
    SAVE_LAYOUT_HERO0_DOWN_L = 4,
    SAVE_LAYOUT_HERO0_DOWN_R = 5,
    SAVE_LAYOUT_HERO0_CURSOR = 6,
    SAVE_LAYOUT_HERO0_UP_L = 7,
    SAVE_LAYOUT_HERO0_UP_R = 8,

    /* Hero 1's weapon list, in the same order. */
    SAVE_LAYOUT_HERO1_ICONS = 9,
    SAVE_LAYOUT_HERO1_NAMES = 10,
    SAVE_LAYOUT_HERO1_DOWN_L = 11,
    SAVE_LAYOUT_HERO1_DOWN_R = 12,
    SAVE_LAYOUT_HERO1_CURSOR = 13,
    SAVE_LAYOUT_HERO1_UP_L = 14,
    SAVE_LAYOUT_HERO1_UP_R = 15,

    SAVE_LAYOUT_HERO0_LIST_FRAME = 16,
    SAVE_LAYOUT_HERO1_LIST_FRAME = 17,
    SAVE_LAYOUT_HERO0_SELECTED = 18, /**< Shown while hero 0 is picked on the home panel. */
    SAVE_LAYOUT_HERO1_SELECTED = 19, /**< Shown while hero 1 is picked on the home panel. */
    SAVE_LAYOUT_HERO0_FIGURE = 20,   /**< Drawn mirrored, as a POLY_FT4. */
    SAVE_LAYOUT_HERO1_FIGURE = 21,
    SAVE_LAYOUT_HERO0_LABEL = 22,
    SAVE_LAYOUT_HERO1_LABEL = 23,
    SAVE_LAYOUT_HERO0_PORTRAIT = 24,
    SAVE_LAYOUT_HERO1_PORTRAIT = 25,
    SAVE_LAYOUT_BACKGROUND = 26,
    SAVE_LAYOUT_ENTRY_COUNT = 27
};

/* The hero caption sits at UV (16, 0), the weapon caption at (0, 16). */
#define SAVE_CAPTION_UV_STEP 0x10
/* Weapon names start this far down the texture; the icons start at V 0. */
#define SAVE_WEAPON_NAMES_V 0x20
#define SAVE_WEAPON_LIST_Y 0x40
#define SAVE_WEAPON_ROW_SHIFT 4
#define SAVE_WEAPON_ROW_HEIGHT (1 << SAVE_WEAPON_ROW_SHIFT)
#define SAVE_WEAPON_VISIBLE_ROWS 7
#define SAVE_WEAPON_SCROLL_MAX ((FIELD_WEAPON_CATEGORY_COUNT - SAVE_WEAPON_VISIBLE_ROWS) * SAVE_WEAPON_ROW_HEIGHT)
#define SAVE_WEAPON_CURSOR_MAX ((SAVE_WEAPON_VISIBLE_ROWS - 1) * SAVE_WEAPON_ROW_HEIGHT)
#define SAVE_WEAPON_SCROLL_FRAMES 4

extern SaveLayoutEntry g_save_layout_table[SAVE_LAYOUT_ENTRY_COUNT];
/** @brief TIM pixel mode and size of an uploaded save-layout texture. */
typedef struct
{
    u32 mode : 3;
    u32 width : 10;
    u32 height : 10;
} SaveLayoutTexControl;

/** @brief A picker TIM and the VRAM position it is uploaded to. */
typedef struct
{
    u16 tex_x;
    u16 tex_y;
    u16 clut_x;
    u16 clut_y;
    TimPrefix* src;
    SaveLayoutTexControl control;
} SaveLayoutTex;

extern SaveLayoutTex g_save_layout_tex_table[];
/** @brief Saved game loaded for a new game. */
extern s32 g_new_game_template[];
/** @brief Saved game loaded when TITLE starts directly in FIELD. */
extern s32 g_field_start_template[];
/** @brief Starting FieldCharacterRecord of the hero with character type 0. */
extern s32 g_hero_template_type_0[SAVED_CHARACTER_SIZE / 4];
/** @brief Starting FieldCharacterRecord of the hero with character type 1. */
extern s32 g_hero_template_type_1[SAVED_CHARACTER_SIZE / 4];

void update_menu_input(void);

#endif
