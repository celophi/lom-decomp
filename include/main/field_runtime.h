#ifndef _FIELD_RUNTIME_H
#define _FIELD_RUNTIME_H

#include "common.h"
#include <libgpu.h>

#define FIELD_ORDERING_TABLE_SIZE 4112
#define FIELD_PRIMITIVE_ARENA_SIZE 15360

/** @brief Ordering table, display setup, and primitive storage for one field frame. */
typedef struct FieldRenderHalf
{
    u_long ordering_table[FIELD_ORDERING_TABLE_SIZE];
    DISPENV disp_env;
    DRAWENV draw_env;
    RECT display_rect;
    u8* primitive_cursor;
    u8 primitive_arena[FIELD_PRIMITIVE_ARENA_SIZE];
    DR_TPAGE draw_mode;
} FieldRenderHalf;

/** @brief Screen position where field_draw_glyph puts the next glyph. */
extern s32 g_text_cursor_x;
extern s32 g_text_cursor_y;
/** @brief CLUT identifier of the field text font. */
extern s32 g_text_clut_base;

s32 run_field_scene(void);
void field_init_text_renderer(FieldRenderHalf* render_buffers);
/**
 * @brief Draw one glyph at the text cursor and advance it (field_runtime_glyph.c).
 * @param character Character code; only the low byte is used.
 * @param ot_depth Ordering-table depth used to link the sprite.
 * @param clut_offset Offset added to the base font CLUT identifier.
 */
void field_draw_glyph(s32 character, s32 ot_depth, s32 clut_offset);

/* FIELD overlay entry points called from the main executable. */
/**
 * @brief Reset the scene selection, the fade state and the text state.
 * @param render_buffers Unused; the main executable passes the field render buffers or NULL.
 */
void field_scene_reset(FieldRenderHalf* render_buffers);
void field_initialize_subsystems(FieldRenderHalf* render_context);
void field_build_frame_commands(FieldRenderHalf* render_half, s32 alternate);
void field_flush_vram_uploads(void);
void field_set_fade_target(s16 red, s16 green, s16 blue, s16 duration);
void field_stop_song(void);
#if defined(VERSION_JP)
void field_draw_frame(s32 alternate_half, FieldRenderHalf* buffer, s32 update_mode);
void field_clear_node_accumulators(s32 update_mode);
#else
void field_draw_frame(s32 alternate_half, FieldRenderHalf* buffer, s32 update_mode, s32 force_unscaled);
void field_clear_node_accumulators(s32 update_mode, s32 force_unscaled);
#endif
void field_restore_entry_music(void);

#endif
