#ifndef _MAIN_H
#define _MAIN_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libapi.h"
#include "psyq/libetc.h"

extern u32 g_gameState;
extern u32 g_previousGameState;
extern u32 g_overlayLoadAddress;
extern u32 g_gameDataBasePtr;

extern u8* D_800351A0;
extern u32 D_8003EC88;
extern s32 D_8003EC8C;
extern u16 D_8003EC90;
extern s32 D_8003EC94;
extern s32 D_8003EC98;
extern s32 D_8003EC9C;
extern s32 D_80042FC4;
extern s32 D_80042FCC;
extern s32 D_80042FD0;
/**
 * @brief Working buffer for the active menu/save layout (main executable .bss).
 *
 * Storage for a MenuLayout record. load_menu_layout copies a full 0xC9A-word
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
extern s32 D_80046FD8;
extern u16 D_80046FDE;
extern s32 D_800473E0;

/*
 * Shared input / frame / script state.
 *
 * These globals live in the main executable's .bss (below the 0x80140000
 * overlay slot) and are read/written by multiple overlays (menu, gname,
 * gover, ...). They are declared here rather than per-overlay so the
 * overlays share a single definition.
 */

/** @brief Global frame counter, advanced once per rendered frame. */
extern s32 g_frame_counter;

/** @brief Base of the primitive-rect scratch buffer (stride 0x4A0 per record). */
extern u8 g_prim_rect_buf[];

/**
 * @brief Controller/pad context object (partial layout).
 *
 * Only the two fields touched by the scripted-input injection path in
 * @ref menu_tick are mapped so far; the rest of the structure is opaque.
 * Both fields together gate whether @c g_pad_input_inject is OR'd into
 * @c g_pad_input on a given frame.
 */
typedef struct
{
    u8  _pad000[0x840];     /**< 0x000: not yet mapped. */
    u8  inject_enable;      /**< 0x840: non-zero allows input injection. */
    u8  _pad841[0x858 - 0x841]; /**< 0x841: not yet mapped. */
    u32 inject_flags;       /**< 0x858: bit 0x80 enables input injection. */
    u8 pad85C[0x24C];
    u32 unkAA8;
} PadContext;

/** @brief Pointer to the controller/pad context object. */
extern PadContext* g_pad_ctx;

/** @brief Number of times the active script repeats on reaching its terminator. */
extern s32 g_script_repeat_count;

/** @brief Active script id; selects a row of @c g_script_table (0 = none). */
extern s32 g_active_script;

/** @brief Current frame's debounced pad button bitmask. */
extern s32 g_pad_input;

/** @brief Extra button bits OR'd into @c g_pad_input when the pad context requests it. */
extern s32 g_pad_input_inject;

/**
 * @brief Active menu/save layout record stored in g_menuLayoutBuffer.
 *
 * Bulk-initialised by load_menu_layout (title overlay), which copies a full
 * 0xC9A-word layout template over it. The main executable also accesses this
 * record directly as @c (MenuLayout*)g_menuLayoutBuffer (which equals
 * @c &g_gameDataBasePtr - 0x5F0); see main.c game state 7, where several
 * header fields are copied verbatim into companion globals.
 *
 * Only mapped fields are named; unmapped spans are kept as padding arrays.
 *
 * @note Partial layout (covers 0x000..0x60C; the buffer itself is larger).
 */
typedef struct
{
    u8  _unk000[0x18];          /**< 0x000: not yet mapped. */
    s32 unk018;                 /**< 0x018: main.c masks 0xFE000000 and ORs in 6. */
    s16 unk01C;                 /**< 0x01C: copied to companion global D_8003EC94. */
    s8  unk01E;                 /**< 0x01E: copied to companion global D_80046FD8. */
    u8  unk01F;                 /**< 0x01F: not yet mapped. */
    u32 unk020;                 /**< 0x020: -> D_80046FDE; index into D_800351A0[]. */
    u16 unk024;                 /**< 0x024: -> D_8003EC90; mode/scene id. */
    u8  unk026;                 /**< 0x026: copied to companion global D_80042FCC. */
    u8  unk027;                 /**< 0x027: copied to companion global D_80042FC4. */
    u32 unk028;                 /**< 0x028: flag word; bits 0xC tested together. */
    u8  _unk02C[0x34 - 0x2C];   /**< 0x02C: not yet mapped. */
    s32 unk034[0xB];            /**< 0x034: 11 s32 slots; cleared per non-selected save slot. */
    u8  _unk060[0xD4 - 0x60];   /**< 0x060: not yet mapped. */
    s16 rng_seed;               /**< 0x0D4: composed random value (rand() based). */
    u8  _unk0D6[0x2E0 - 0xD6];  /**< 0x0D6: not yet mapped. */
    s32 mode_flags;             /**< 0x2E0: state bitfield; bit 0 = "continue mode". */
    u8  _unk2E4[0x608 - 0x2E4]; /**< 0x2E4: not yet mapped. */
    s32 slot_flags;             /**< 0x608: low 7 bits + bit 0 (continue). */
} MenuLayout;                   /* partial; sizeof so far == 0x60C */

void __main(void);
void _bu_init(void);
s32 func_80015C48(void);
u32 func_8004FC74(void);
u32 func_8004FC8C(u32);
void field_scene_reset(u32);
void func_800A3534(void);
u32 run_overlay(u32, u32, u32, s32, s32, u32, s32);
s32 func_801400C4(void);
void srand(u_int param_1);

#endif