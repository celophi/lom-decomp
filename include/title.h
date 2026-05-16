#ifndef _TITLE_H
#define _TITLE_H

#include "common.h"
#include "akao.h"
#include "pad.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

typedef struct
{
    u16 unk0;
    u16 unk2;
    u32 unk4;
    u32 unk8;
    u32 unkC;
} S_801ED480;

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
    u_long otag_buffer2[0x1000];

} MenuContext; /* 0xBCCC total */

/**
 * @brief Active menu/save layout record stored in g_menuLayoutBuffer.
 *
 * Bulk-initialised by LoadMenuLayout, which copies a full 0xC9A-word layout
 * table over it. Only a few fields are mapped so far; the unmapped spans are
 * kept as padding arrays. Extend this struct as fields are identified rather
 * than widening raw casts at call sites.
 *
 * @note Partial layout. g_menuLayoutBuffer is still declared @c u8[]; cast to
 *       @c MenuLayout* only at call sites that have been converted and
 *       asm-verified (currently load_sub_menu_layout).
 */
typedef struct
{
    u8  _unk000[0xD4];          /**< 0x000: layout data, not yet mapped. */
    s16 rng_seed;               /**< 0x0D4: composed random value (rand() based). */
    u8  _unk0D6[0x2E0 - 0xD6];  /**< 0x0D6: not yet mapped. */
    s32 mode_flags;             /**< 0x2E0: state bitfield; bit 0 = "continue mode". */
} MenuLayout;

/**
 * @brief Working buffer for the active menu/save layout (main executable .bss).
 *
 * Storage for a MenuLayout record. LoadMenuLayout copies a full 0xC9A-word
 * layout table into this buffer; load_sub_menu_layout overwrites a 0x94-word
 * sub-region whose base is the separately-named g_gameDataBasePtr (offset
 * 0x5F0 within this same buffer). Fields accessed so far by the title overlay:
 *   +0x0D4  s16  composed random value (seed) written from rand()
 *   +0x2E0  s32  mode flags; bit 0 = "continue mode"
 *   +0x608  s32  slot flags (low 7 bits cleared, bit 0 set for continue)
 *
 * @note Kept as @c u8[] so existing byte-granular pointer arithmetic compiles
 *       to unchanged codegen; cast to @c MenuLayout* per converted call site.
 */
extern u8 g_menuLayoutBuffer[];
extern s32 D_80042FB4;
extern u8 g_titleSelectedItem;
extern s32 D_8003EC9C;
extern s32 g_titleMenuExitState;
extern u32 g_previousGameState;
/**
 * Base address of the AKAO instrument/sample bank loaded by LoadTitleAudioBank
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
extern u8 D_8007FD2C[];

/**
 * Timer used to implement input repeating (auto-repeat).
 * Controls the delay before a held button begins triggering actions rapidly.
 * Same semantics as the identically-named symbol in the CHECKPS overlay
 * (separate copy, different address).
 */
extern s32 g_inputRepeatTimer;
extern s32 g_lastInputState;
extern u32 D_800522E8[3];
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
extern u8 D_800F9BC4[];
extern u8 D_800F9AED;
/**
 * @brief One entry in the 27-element save-slot UI layout table (D_800F993C).
 *
 * The layout has 0x1B entries × 0x18 bytes = 0x288 bytes total.
 *
 * @note D_800F993C is kept as @c u8[] so that all existing raw byte-offset
 *       accesses in the function bodies compile to unchanged codegen.
 *       Cast to @c SaveLayoutEntry* at call sites that benefit from named
 *       field access.
 */
typedef struct {
    u8  flags;    /**< +0x00: bit0=apply_slide, bit1=semi_transparent, bits2-3=abr */
    u8  type;     /**< +0x01: prim type: 0=skip, 2=TILE, 3=POLY_FT4, 4=SPRT, other=glyph */
    u8  tex_slot; /**< +0x02: index into D_800F97FC[] tex table (stride 0x10) */
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

/* 0x1B (27) entries; kept as u8[] to preserve byte-granular pointer arithmetic */
extern u8 D_800F993C[0x288];
extern u8 D_800F97FC[];
extern u8 D_800F98AC[];
extern u8 D_800F98F4[];
extern s32 D_800F9E84;
extern s16 D_8003EC90;
extern s32 D_800FEF40;
extern s16 D_80046FDE;
extern s32 D_80042FC4;
/** Sub-menu layout table copied by load_sub_menu_layout for a new game. */
extern s32 g_subMenuLayoutDefault[0x94];
/** Sub-menu layout table copied by load_sub_menu_layout when resuming a save. */
extern s32 g_subMenuLayoutContinue[0x94];
extern s32 g_gameDataBasePtr;

extern FadeCurrent g_fadeCurrent;
extern FadeTarget g_fadeTarget;

extern void akao_cmd_c0(undefined4 param_1, u_int param_2);
extern void akao_play_song(s32 arg0);
extern void akao_play_sfx(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

#endif
