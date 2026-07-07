#ifndef _TITLE_H
#define _TITLE_H

#include "common.h"
#include "main.h"
#include "akao.h"
#include "pad.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "scene_state.h"

/**
 * Current screen-fade colour. RGB only; the fade-target struct carries the
 * step counter. Counterpart to g_fadeCurrent in the CHECKPS overlay.
 */
typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
} FadeCurrent;

/**
 * Target colour and remaining frames for the screen-fade interpolation.
 * Counterpart to g_fadeTarget in the CHECKPS overlay (which uses a single
 * FadeColor struct for both; here current/target have distinct sizes
 * because g_fadeCurrent is followed immediately by another global).
 */
typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
    s32 steps;
} FadeTarget;

typedef struct
{
    char _pad[0x40];
    u_long otag_buffer[0x1000]; /* 0x0040 */
    DISPENV disp_env;           /* 0x4040 */
    DRAWENV draw_env;           /* 0x4054 */
    char _pad2[8];              /* 0x40B0 */
    u_long prim_buffer[0x1000]; /* 0x40B8 */
    u_long* next_prim_ptr;      /* 0x80B8 */
    char _pad3[0x3C10];         /* 0x80BC */

    char _pad4[0x40];
    u_long otag_buffer2[0x1000]; /* 0xBD0C */
    DISPENV disp_env2;           /* 0xFD0C */
    DRAWENV draw_env2;           /* 0xFD20 */
    char _pad5[8];               /* 0xFD7C */

} MenuContext; /* 0xFD84 total */

/* MenuLayout and g_menuLayoutBuffer are declared in main.h (shared). */
extern s32 D_80042FB4;
extern u8 g_titleSelectedItem;
extern s32 g_titleMenuExitState;
/**
 * Base address of the AKAO instrument/sample bank loaded by load_title_audio_bank
 * (always 0x8013C000). Passed to akao_register_bank to register it with the
 * audio driver.
 */
extern s32 g_titleAudioBankBase;
extern unsigned char D_8003ECA0;
extern s32 g_titleIdleCountdown;
extern s32 g_debouncedInput;
extern u8 g_titleMenuItemFlags[];
extern u8 g_titleVisibleItemRank;
extern u8 g_titleAnimFrame;
/**
 * 4-entry CLUT-index table that blinks the title-menu cursor; indexed by
 * (g_titleAnimFrame >> 2) & 3 in render_title_menu_items.
 */
extern u8 g_cursorBlinkPalette[];

/**
 * Timer used to implement input repeating (auto-repeat).
 * Controls the delay before a held button begins triggering actions rapidly.
 * Same semantics as the identically-named symbol in the CHECKPS overlay
 * (separate copy, different address).
 */
extern s32 g_inputRepeatTimer;
extern s32 g_lastInputState;
/**
 * Self-relative offset table for the two title-menu TIMs uploaded by
 * init_title_menu_state: entries [1] and [2] are byte offsets from the table's
 * own base to each TIM ([0] is unused).
 */
extern u32 g_titleMenuTimTable[3];
extern u8 D_801ED600[];
extern s32 g_slotSlideX;
extern s32 g_slotSlideY;
extern s32 g_slotSelectedIndex;
extern s32 g_slotSlideFrames;
extern s32 g_slotHighlightX;
extern s32 g_slotHighlightTargetX;
extern s32 g_slotSlideXLerped;
extern s32 g_slotSlideYLerped;
extern s32 g_slotHighlightFrames;
extern u8 D_80043618[0x40];
/**
 * Source array of save-slot records (stride 0x40); handle_save_slot_input copies
 * the record selected by g_slotSelectedIndex into D_80043618.
 */
extern u8 g_saveSlotData[];
extern u8 D_800F9AED;
/**
 * @brief One entry in the 27-element save-slot UI layout table (g_saveLayoutTable).
 *
 * The layout has 0x1B entries × 0x18 bytes = 0x288 bytes total.
 *
 * @note g_saveLayoutTable is kept as @c u8[] so that all existing raw byte-offset
 *       accesses in the function bodies compile to unchanged codegen.
 *       Cast to @c SaveLayoutEntry* at call sites that benefit from named
 *       field access.
 */
typedef struct {
    u8  flags;    /**< +0x00: bit0=apply_slide, bit1=semi_transparent, bits2-3=abr */
    u8  type;     /**< +0x01: prim type: 0=skip, 2=TILE, 3=POLY_FT4, 4=SPRT, other=glyph */
    u8  tex_slot; /**< +0x02: index into g_saveLayoutTexTable[] tex table (stride 0x10) */
    u8  pad;      /**< +0x03 */
    s16 x;        /**< +0x04: screen base X (POLY_FT4, SPRT, glyph) */
    s16 y;        /**< +0x06: screen base Y */
    s16 tile_x;   /**< +0x08: screen X for TILE (slideX always added) */
    s16 tile_y;   /**< +0x0A: screen Y for TILE */
    u16 u0;       /**< +0x0C: initial U texture coordinate (glyph strip) */
    u16 v0;       /**< +0x0E: initial V; animated by AnimateSaveSlotPanel for highlight entries */
    u16 width;    /**< +0x10: TILE.w / glyph total pixel width (chunked at 128 px) */
    u16 height;   /**< +0x12: TILE.h / glyph per-chunk sprite height */
    u8  unk14[4]; /**< +0x14: TODO */
} SaveLayoutEntry;             /* sizeof == 0x18 */

/* Home U/V texture coordinate the panel's primary entry resets to. */
#define SAVE_SLOT_HOME_V 0x10
/* Vertical span of the slot highlight bar (added to g_slotHighlightX to get
 * the bottom V coordinate). */
#define SAVE_HIGHLIGHT_SPAN 0x20
/* Scroll-window width when the panel is re-homed (minimum value; see
 * AnimateSaveSlotPanel which adds the pan offset to this base). */
#define SAVE_SCROLL_WIDTH_HOME 0x40

/* 0x1B (27) entries; kept as u8[] to preserve byte-granular pointer arithmetic */
extern u8 g_saveLayoutTable[0x288];
/**
 * Texture-descriptor table for the save-slot layout: 11 entries of stride 0x10,
 * each holding VRAM coords plus the source TIM pointer/control word uploaded by
 * upload_save_layout_textures. Indexed by SaveLayoutEntry::tex_slot.
 */
extern u8 g_saveLayoutTexTable[];
extern u8 D_800F98AC[];
extern u8 D_800F98F4[];
/** Menu-layout template copied into g_menuLayoutBuffer for the default menu. */
extern MenuLayout g_menuLayoutTemplateDefault;
/** Menu-layout template copied into g_menuLayoutBuffer for the alternate menu. */
extern MenuLayout g_menuLayoutTemplateAlt;
/** Sub-menu layout table copied by load_sub_menu_layout for a new game. */
extern s32 g_subMenuLayoutDefault[0x94];
/** Sub-menu layout table copied by load_sub_menu_layout when resuming a save. */
extern s32 g_subMenuLayoutContinue[0x94];
/* g_scene_mode, g_music_track_index, g_layout_flag, g_gameDataBasePtr are declared in main.h. */

extern FadeCurrent g_fadeCurrent;
extern FadeTarget g_fadeTarget;

extern void akao_cmd_c0(undefined4 param_1, u_int param_2);
extern void akao_play_song(s32 arg0);
extern void akao_play_sfx(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

#endif
