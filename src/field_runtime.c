#include "field_scene_transition.h"
#include "field_runtime.h"
#include "cdrom.h"
#include "controller.h"
#include "overlay_memory.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "scene_state.h"
#include "display.h"
#include "akao_cmd.h"
#include "controller_internal.h"
#include "game_state.h"
#include "sdk/libetc.h"

#define FIELD_PROJECTION_DISTANCE 1500
#define FIELD_ENTRY_FADE_FRAMES 30
#define FIELD_FADE_NEUTRAL 256
#define FIELD_TEXT_IMAGE_RESOURCE 1500
#define FIELD_TEXT_IMAGE_X 288
#define FIELD_TEXT_IMAGE_Y 480
#define FIELD_TEXT_CLUT_X 256
#define FIELD_TEXT_CLUT_Y 511
#define FIELD_TEXT_TPAGE getTPage(0, 1, 256, 256)

/** @brief Field allocation cursor, text state, and primitive-buffer bases. */
typedef struct
{
    u8* allocation_cursor;
    void* text_window_configs;
    u8* timed_text;
    u8* primitive_buffers[2];
} FieldWorkspace;

#define FIELD_WORKSPACE ((FieldWorkspace*)0x801ED000)

extern void field_set_fade_target(s16, s16, s16, s16);

extern void field_stop_song(void);

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
extern FieldRenderHalf* g_field_current_render_half;
extern void* g_field_primitive_cursor;
extern s32 g_field_force_two_primitives;
extern s32 g_field_draw_count;
extern s32 g_text_clut_base;
extern s32 g_text_cursor_x;
extern s32 g_text_cursor_y;

void field_scene_reset();
void field_draw_frame(s32 alternate_half, FieldRenderHalf* render_half, s32 draw_count, s32 force_two_primitives);
void field_clear_node_accumulators(s32 draw_count, s32 force_two_primitives);

void field_run_frame_loop(FieldRenderHalf* render_buffers);
void field_init_display(FieldRenderHalf* render_buffers);

/**
 * @brief Run the field overlay's top-level scene loop until a state
 *        transition is requested, then return the next game state.
 * @return Pending state when below GAME_STATE_WORLD_SELECT, otherwise GAME_STATE_WORLD_MAP.
 * @see decomp.me (100%) https://decomp.me/scratch/CPx5C
 */
s32 run_field_scene(void)
{
    FieldRenderHalf* render_buffers;
    s32 next_state;
    SceneState* scene_state = SCENE_STATE;
    render_buffers = get_field_render_buffers();
    field_init_display(render_buffers);
    scene_state->map_id = 0;
    scene_state->object_index = 0;
    scene_state->camera_x = 0;
    scene_state->camera_y = 0;
    scene_state->camera_z = 0;
    do
    {
        next_state = FIELD_ENTRY_FADE_FRAMES;
        g_field_scene_request_pending = 0;
        field_set_scene_parameters(g_scene_mode, g_field_entry_flag, g_field_scene_config, g_layout_flag, g_layout_option, g_layout_sub_mode);
        field_set_fade_target(FIELD_FADE_NEUTRAL, FIELD_FADE_NEUTRAL, FIELD_FADE_NEUTRAL, next_state);
        field_run_frame_loop(render_buffers);
    } while (g_pending_game_state == 0);
    field_stop_song();
    akao_cmd_f0();
    akao_cmd_f1();
    next_state = g_pending_game_state;
    if (next_state < GAME_STATE_WORLD_SELECT)
    {
        return next_state;
    }
    return GAME_STATE_WORLD_MAP;
}

/**
 * @brief Per-frame field render loop; runs until a state transition is
 *        requested via g_pending_game_state.
 * @param render_buffers Both field render buffers.
 * @see decomp.me (100%) https://decomp.me/scratch/ViJdW
 */
void field_run_frame_loop(FieldRenderHalf* render_buffers)
{
    RECT vram_rect;
    FieldRenderHalf* draw_half;
    ControllerState* controllers;
    FieldWorkspace* field_heap;
    s32 is_alt_half;
    FieldRenderHalf* next_half;
    u8* primitive_cursor;
    vram_rect.x = 0;
    vram_rect.y = 0;
    vram_rect.w = SCREEN_WIDTH;
    vram_rect.h = VRAM_BACK_DISP_Y + SCREEN_HEIGHT;
    field_heap = FIELD_WORKSPACE;
    controllers = CONTROLLER_STATE;
    ClearImage(&vram_rect, 0, 0, 0);
    draw_half = render_buffers;
    ClearOTagR(draw_half->ordering_table, FIELD_ORDERING_TABLE_SIZE);
    ClearOTagR(draw_half[1].ordering_table, FIELD_ORDERING_TABLE_SIZE);
    VSync(0);
    PutDispEnv(&draw_half->disp_env);
    update_controllers();
    SetDispMask(1);
    do
    {
        field_update_scene();
        g_field_current_render_half = draw_half;
        g_field_primitive_cursor = draw_half->primitive_arena;

        ClearOTagR(draw_half->ordering_table, FIELD_ORDERING_TABLE_SIZE);
        if (draw_half == render_buffers)
        {
            primitive_cursor = field_heap->primitive_buffers[0];
        }
        else
        {
            primitive_cursor = field_heap->primitive_buffers[1];
        }
        draw_half->primitive_cursor = primitive_cursor;
        field_clear_node_accumulators(g_field_draw_count, g_field_force_two_primitives);
        is_alt_half = draw_half != render_buffers;
        field_build_frame_commands(draw_half, is_alt_half);
        if (g_pending_game_state == 0)
        {
            VSync(1);
            field_draw_frame(is_alt_half, draw_half, g_field_draw_count, g_field_force_two_primitives);
            VSync(1);
            DrawSync(0);
            set_controller_vsync_interval(2);
            VSync(2);
            next_half = render_buffers;
            if (draw_half == render_buffers)
            {
                next_half = &draw_half[1];
            }
            draw_half = next_half;
            PutDispEnv(&draw_half->disp_env);
            PutDrawEnv(&draw_half->draw_env);
            field_flush_vram_uploads();
            DrawOTag(&g_field_current_render_half->ordering_table[FIELD_ORDERING_TABLE_SIZE - 1]);
            update_controllers();
            cdrom_process_state();
        }
    } while (g_pending_game_state == 0);
    controllers->ports[1].actuators_enabled = 0;
    controllers->ports[0].actuators_enabled = 0;
    reset_controller_vsync_state();
    akao_cmd_f0();
    akao_cmd_f1();
    DrawSync(0);
    VSync(0);
    SetDispMask(0);
}

/**
 * @brief Field overlay one-time init: projection geometry, double-buffer
 *        display/draw environments, and initial scene state reset.
 * @param render_buffers Both field render buffers.
 * @see decomp.me (100%) https://decomp.me/scratch/JAUtV
 */
void field_init_display(FieldRenderHalf* render_buffers)
{
    RECT vram_rect;

    SetGeomScreen(FIELD_PROJECTION_DISTANCE);
    SetGeomOffset(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    render_buffers[0].display_rect.x = 0;
    render_buffers[0].display_rect.y = 0;
    render_buffers[0].display_rect.w = SCREEN_WIDTH;
    render_buffers[0].display_rect.h = SCREEN_HEIGHT;
    render_buffers[1].display_rect.y = VRAM_BACK_DISP_Y;
    render_buffers[1].display_rect.x = 0;
    render_buffers[1].display_rect.w = SCREEN_WIDTH;
    render_buffers[1].display_rect.h = SCREEN_HEIGHT;

    vram_rect.x = 0;
    vram_rect.y = 0;
    vram_rect.w = VRAM_WIDTH;
    vram_rect.h = VRAM_HEIGHT;
    ClearImage(&vram_rect, 0, 0, 0);

    SetDefDispEnv(&render_buffers[0].disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&render_buffers[1].disp_env, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&render_buffers[0].draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&render_buffers[1].draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    render_buffers[1].draw_env.dtd = 0;
    render_buffers[0].draw_env.dtd = 0;

    field_restore_entry_music();
    g_pending_game_state = 0;
    field_initialize_subsystems(render_buffers);
    g_field_draw_count = 0;
    field_scene_reset(render_buffers);
}

/**
 * @brief Initialize the field text renderer, load its VRAM atlas, and seed the
 *        draw-mode packet at the end of each double-buffered render half.
 * @param render_buffers Field render context containing both render halves.
 * @see decomp.me (100%) https://decomp.me/scratch/v32hU
 */
void field_init_text_renderer(FieldRenderHalf* render_buffers)
{
    s16 font_vram_layout[4];

    g_text_cursor_y = 0;
    g_text_cursor_x = 0;
    font_vram_layout[0] = FIELD_TEXT_IMAGE_X;
    font_vram_layout[1] = FIELD_TEXT_IMAGE_Y;
    font_vram_layout[2] = FIELD_TEXT_CLUT_X;
    font_vram_layout[3] = FIELD_TEXT_CLUT_Y;
    field_load_vram_resource(FIELD_TEXT_IMAGE_RESOURCE, font_vram_layout);
    cdrom_wait_queue_empty();
    g_text_clut_base = getClut(FIELD_TEXT_CLUT_X, FIELD_TEXT_CLUT_Y);
    setDrawTPage(&render_buffers[0].draw_mode, 0, 1, FIELD_TEXT_TPAGE);
    setDrawTPage(&render_buffers[1].draw_mode, 0, 1, FIELD_TEXT_TPAGE);
}
