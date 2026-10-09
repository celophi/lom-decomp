#ifndef _MAIN_H
#define _MAIN_H

#include "common.h"
#include "main/game_state.h"
#include "common/saved_game.h"
#include "main/field_entry.h"
#include <libgte.h>
#include <libgpu.h>
#include <libapi.h>
#include <libetc.h>

/** @brief Value set by field pair opcode 0x49 and cleared at boot. */
extern s32 g_script_pair_value_49;

/** @brief Countdown timer for delayed music/SFX trigger on field entry. */
extern s32 g_field_audio_timer;
/** @brief The running game's save compatibility tag (see SAVE_TAG_ANY in saved_game.h). */
extern s32 g_save_compatibility_tag;
/** @brief Index into g_music_track_table[] selecting the current music track. */
extern u16 g_music_track_index;

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

/** @brief Forward selection steps applied when menu scripts 1-3 terminate. */
extern s32 g_script_repeat_count;

/** @brief Active script id; selects a row of @c g_script_table (0 = none). */
extern s32 g_active_script;

/** @brief Current frame's debounced pad button bitmask. */
extern s32 g_pad_input;

/** @brief Extra button bits OR'd into @c g_pad_input when the pad context requests it. */
extern s32 g_pad_input_inject;

/** @brief Initialize the game and dispatch overlays forever. */
void main_game_loop(void);

#endif
