#ifndef LOM_TITLE_INTERNAL_H
#define LOM_TITLE_INTERNAL_H

#include "overlays/title/title.h"
#include "main/cd_resources.h"
#include "main/display.h"
#include "common/gpu_packet.h"
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
extern s32 g_slot_highlight_x;
extern s32 g_slot_highlight_target_x;
extern s32 g_slot_slide_x_lerped;
extern s32 g_slot_slide_y_lerped;
extern s32 g_slot_highlight_frames;
/** @brief Starting weapon of each weapon category; the chosen one becomes the hero's weapon. */
extern FieldItemRecord g_starting_weapon_records[FIELD_WEAPON_CATEGORY_COUNT];
/** @brief One primitive of the new-game picker screen. */
typedef struct
{
    u8 flags; /**< bit 0: follows the panel slide, bit 1: semi-transparent, bits 2-3: blend mode */
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

#define SAVE_SLOT_HOME_V 0x10
#define SAVE_HIGHLIGHT_SPAN 0x20
#define SAVE_SCROLL_WIDTH_HOME 0x40

/** @brief Save-slot picker entries, including the mode selector at index 18. */
extern SaveLayoutEntry g_save_layout_table[27];
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
    s16 tex_x;
    s16 tex_y;
    s16 clut_x;
    s16 clut_y;
    u8* src;
    SaveLayoutTexControl control;
} SaveLayoutTex;

extern SaveLayoutTex g_save_layout_tex_table[];
/** @brief Saved game loaded for a new game. */
extern u32 g_new_game_template[];
/** @brief Saved game loaded when TITLE starts directly in FIELD. */
extern u32 g_field_start_template[];
/** @brief Starting FieldCharacterRecord of the hero with character type 0. */
extern s32 g_hero_template_type_0[SAVED_CHARACTER_SIZE / 4];
/** @brief Starting FieldCharacterRecord of the hero with character type 1. */
extern s32 g_hero_template_type_1[SAVED_CHARACTER_SIZE / 4];

void update_menu_input(void);

#endif
