#ifndef _DECOMP7_H
#define _DECOMP7_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "scene_state.h"
#include "display.h"

/** @brief Fixed-address scene-selection state block; see S_801ED480. */
#define SCENE_STATE ((S_801ED480*)0x801ED480)

/** @brief One 0x7CC4-byte half of the field overlay's double-buffered render context. */
typedef struct obj_struct
{
  u_long otag[0x1010]; /**< 0x0000 ordering table (0x1010 entries, cleared by ClearOTagR) */
  DISPENV disp_env;   /**< 0x4040 */
  DRAWENV draw_env;   /**< 0x4054 */
  RECT display_rect;  /**< 0x40B0 display rectangle associated with this half */
  u32 primitive_cursor; /**< 0x40B8 current end of this half's primitive arena */
  u8 primitive_arena[0x7CBC - 0x40BC];
  DR_TPAGE draw_mode; /**< 0x7CBC one-command drawing-page packet */
} FieldRenderHalf;

/** @brief Offset-accurate view spanning both halves of a field render context. */
typedef struct FieldRenderContextLayout {
    u8   _pad0[0x406A];                 /* padding up to offset 0x406A */
    u8   front_draw_dither;             /* offset 0x406A: front DRAWENV.dtd */
    u8   _pad1[0x40B0 - 0x406B];        /* padding to offset 0x40B0 */
    s16  front_display_x;               /* offset 0x40B0 */
    s16  front_display_y;               /* offset 0x40B2 */
    s16  front_display_width;           /* offset 0x40B4 */
    s16  front_display_height;          /* offset 0x40B6 */
    u8   _pad2[0xBD2E - 0x40B8];        /* padding to offset 0xBD2E */
    u8   back_draw_dither;              /* offset 0xBD2E: back DRAWENV.dtd */
    u8   _pad3[0xBD74 - 0xBD2F];        /* padding to offset 0xBD74 */
    s16  back_display_x;                /* offset 0xBD74 */
    s16  back_display_y;                /* offset 0xBD76 */
    s16  back_display_width;            /* offset 0xBD78 */
    s16  back_display_height;           /* offset 0xBD7A */
} FieldRenderContextLayout;

/** @brief Offset view of the draw-mode packets terminating both render halves. */
typedef struct FieldDrawModeLayout {
    u8   _pad0[0x7CBF];                /* padding up to offset 0x7CBF */
    u8   front_packet_length;          /* offset 0x7CBF: high byte of DR_TPAGE.tag */
    u32  front_draw_mode_command;      /* offset 0x7CC0 */
    u8   _pad2[0xF983 - 0x7CC4];       /* padding to offset 0xF983 */
    u8   back_packet_length;           /* offset 0xF983: high byte of DR_TPAGE.tag */
    u32  back_draw_mode_command;       /* offset 0xF984 */
} FieldDrawModeLayout;

extern u32 *get_field_render_buffers(void);
extern void akao_cmd_f0(void);
extern void akao_cmd_f1(void);
void field_run_frame_loop(FieldRenderHalf*);
void field_init_display(FieldRenderHalf*);
void field_init_text_renderer(FieldRenderHalf*);
extern void field_set_fade_target(s16, s16, s16, s16);
extern void field_set_scene_parameters(s32, s32, u32, s32, s32, s32);
extern void field_stop_song(void);
extern void field_update_scene(void);
extern void field_build_frame_commands(FieldRenderHalf*, s32);
extern void field_initialize_subsystems(FieldRenderHalf*);
extern void field_flush_vram_uploads(void);
extern void field_load_vram_resource(s32, s16*);
extern void field_restore_entry_music(void);
extern s32 g_scene_mode;
extern s32 g_field_entry_flag;
extern u32 g_field_scene_config;
extern s32 g_layout_flag;
extern s32 g_layout_option;
extern s32 g_layout_sub_mode;
extern s32 g_pending_game_state;
extern s32 g_field_scene_request_pending;
extern void *g_field_current_render_half;
extern void *g_field_primitive_cursor;
extern s32 g_field_force_two_primitives;
extern s32 g_field_draw_count;
extern s32 g_text_atlas_base;
extern s32 g_text_cursor_x;
extern s32 g_text_cursor_y;

#endif
