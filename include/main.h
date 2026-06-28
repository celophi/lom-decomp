#ifndef _MAIN_H
#define _MAIN_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libapi.h"
#include "psyq/libetc.h"

/**
 * @brief Top-level game state machine values stored in g_gameState.
 *
 * Main() drives the game through these states in an infinite loop. States
 * 0/9/10 share a code path (enter field, optionally with attract movies).
 * State 4 is a transient that is immediately redirected to STATE_TITLE.
 */
#define GAME_STATE_FIELD        0   /**< Enter field overlay (attract / demo / normal). */
#define GAME_STATE_WORLD_MAP    1   /**< Enter world map overlay (WMAP.BIN). */
#define GAME_STATE_TITLE        2   /**< Enter title screen (TITLE.BIN). */
#define GAME_STATE_GNAME        3   /**< Enter GNAME overlay (name entry, within field). */
#define GAME_STATE_WORLD_SELECT 5   /**< Enter world select overlay (WSEL.BIN). */
#define GAME_STATE_MENU_LOAD    7   /**< Enter menu/load overlay (save / continue). */
#define GAME_STATE_INTRO_MOVIE  8   /**< Play intro movie, then transition to title. */
#define GAME_STATE_ATTRACT_1    9   /**< Attract demo: play movie 1, then field. */
#define GAME_STATE_ATTRACT_2    10  /**< Attract demo: play movies 2-4, then field. */
#define GAME_STATE_NONE         0xFF /**< Sentinel: no previous state (init). */

extern u32 g_gameState;
extern u32 g_previousGameState;
extern u32 g_overlayLoadAddress;
extern u32 g_gameDataBasePtr;

/** @brief Table of music/song resource IDs indexed by g_music_track_index. */
extern u8 g_music_track_table[];
extern u32 D_8003EC88;              /**< TODO: unknown; read by func_8009AFE0, never written in C */
extern s32 D_8003EC8C;              /**< TODO: unknown; one-time init to 0xB */
/** @brief Current scene/mode identifier (0, 0xD for default menu template). */
extern u16 g_scene_mode;
/** @brief Option/parameter word from MenuLayout; -1 = unset. */
extern s32 g_layout_option;
/** @brief Countdown timer for delayed music/SFX trigger on field entry. */
extern s32 g_field_audio_timer;
/** @brief Selected save slot index (7 = init, 0xFF = no save selected). */
extern s32 g_save_slot_index;
/** @brief Menu layout configuration flag byte (from MenuLayout.layout_flags). */
extern s32 g_layout_flag;
/** @brief Field-entry behavior flag (from MenuLayout.field_flags). Cleared in field-entry states. */
extern s32 g_field_entry_flag;
extern s32 D_80042FD0;              /**< TODO: unknown; one-time init to 0x13, sits at g_menuLayoutBuffer - 8 */
/**
 * @brief Working buffer for the active menu/save layout (main executable .bss).
 *
 * Storage for a MenuLayout record. load_menu_layout copies a full 0xC9A-word
 * layout table into this buffer; load_sub_menu_layout overwrites a 0x94-word
 * sub-region whose base is the separately-named g_gameDataBasePtr (offset
 * 0x5F0 within this same buffer). Fields accessed so far by the title overlay:
 *   +0x0D4  s16  rng_seed - composed random value (seed) written from rand()
 *   +0x2E0  s32  mode_flags - bit 0 = "continue mode"
 *   +0x608  s32  slot_flags - low 7 bits cleared, bit 0 set for continue
 *
 * Companion globals shadow several MenuLayout fields for use by the main loop
 * and field overlay: g_scene_mode, g_layout_option, g_field_audio_timer,
 * g_save_slot_index, g_layout_flag, g_field_entry_flag, g_layout_sub_mode,
 * g_music_track_index.
 *
 * @note Kept as @c u8[] so existing byte-granular pointer arithmetic compiles
 *       to unchanged codegen; cast to @c MenuLayout* per converted call site.
 */
extern u8 g_menuLayoutBuffer[];
/** @brief Signed sub-mode byte from MenuLayout.sub_mode; -1 = unset. */
extern s32 g_layout_sub_mode;
/** @brief Index into g_music_track_table[] selecting the current music track. */
extern u16 g_music_track_index;
/** @brief TODO: unknown; one-time init to 0. */
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
    u8  _padAAC[0x29D7 - 0xAAC];  /**< 0xAAC: not yet mapped. */
    s8  unk29D7;                  /**< 0x29D7: when (low 7 bits == 4) the entered name is
                                   *           written into the 0x14C-stride history table. */
    u8  _pad29D8[0x2B0C - 0x29D8];/**< 0x29D8: not yet mapped. */
    u8  unk2B0C_arr[3][0x14C];    /**< 0x2B0C: three 0x14C-byte name-history slots. */
    u32 unk2EF0;                  /**< 0x2EF0: slot index into @c unk2EF4_arr. */
    u8  unk2EF4_arr[3][0x60];     /**< 0x2EF4: three 0x60-byte name-history slots. */
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
    s16 option_id;                 /**< 0x01C: copied to companion global g_layout_option. */
    s8  sub_mode;                 /**< 0x01E: copied to companion global g_layout_sub_mode. */
    u8  unk01F;                 /**< 0x01F: not yet mapped. */
    u32 music_track;                 /**< 0x020: -> g_music_track_index; index into g_music_track_table[]. */
    u16 scene_mode;                 /**< 0x024: -> g_scene_mode; mode/scene id. */
    u8  field_flags;                 /**< 0x026: copied to companion global g_field_entry_flag. */
    u8  layout_flags;                 /**< 0x027: copied to companion global g_layout_flag. */
    u32 unk028;                 /**< 0x028: flag word; bits 0xC tested together. */
    u8  _unk02C[0x34 - 0x2C];   /**< 0x02C: not yet mapped. */
    s32 save_slot_data[0xB];            /**< 0x034: 11 s32 slots; cleared per non-selected save slot. */
    u8  _unk060[0xD4 - 0x60];   /**< 0x060: not yet mapped. */
    s16 rng_seed;               /**< 0x0D4: composed random value (rand() based). */
    u8  _unk0D6[0x2E0 - 0xD6];  /**< 0x0D6: not yet mapped. */
    s32 mode_flags;             /**< 0x2E0: state bitfield; bit 0 = "continue mode". */
    u8  _unk2E4[0x608 - 0x2E4]; /**< 0x2E4: not yet mapped. */
    s32 slot_flags;             /**< 0x608: low 7 bits + bit 0 (continue). */
} MenuLayout;                   /* partial; sizeof so far == 0x60C */

void __main(void);
void _bu_init(void);
u32 *func_80015C48(void);
u32 func_8004FC8C(u32);
void field_scene_reset(u32);
void func_800A3534(void);
u32 run_overlay(u32, u32, u32, s32, s32, u32, s32);
s32 func_801400C4(void);
void srand(u_int param_1);
s32 func_8004FC74(s32);

#endif