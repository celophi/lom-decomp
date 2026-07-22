#include "field_runtime.h"

/**
 * @brief Run the field overlay's top-level scene loop until a state
 *        transition is requested, then return the next game state.
 * @return Next g_gameState value: the raw g_pending_game_state exit code if it is
 *         a valid state (< 5), otherwise clamped to GAME_STATE_WORLD_MAP.
 * @see decomp.me (100%) https://decomp.me/scratch/CPx5C
 */
s32 run_field_scene(void)
{
    FieldRenderHalf* render_ctx;
    s32 next_state;
    S_801ED480* scene_state = SCENE_STATE;
    render_ctx = (FieldRenderHalf*)get_field_render_buffers();
    field_init_display(render_ctx);
    scene_state->map_id = 0;
    scene_state->object_index = 0;
    scene_state->unk4 = 0;
    scene_state->unk8 = 0;
    scene_state->unkC = 0;
    do
    {
        next_state = 0x1E;
        g_field_scene_request_pending = 0;
        field_set_scene_parameters(g_scene_mode, g_field_entry_flag, g_field_scene_config, g_layout_flag, g_layout_option, g_layout_sub_mode);
        field_set_fade_target(0x100, 0x100, 0x100, next_state);
        field_run_frame_loop(render_ctx);
    } while (g_pending_game_state == 0);
    field_stop_song();
    akao_cmd_f0();
    akao_cmd_f1();
    next_state = g_pending_game_state;
    if (next_state < 5)
    {
        return next_state;
    }
    return 1;
}

/**
 * @brief Per-frame field render loop; runs until a state transition is
 *        requested via g_pending_game_state.
 * @param render_ctx Field render context (two 0x7CC4-byte frame buffers).
 * @see decomp.me (100%) https://decomp.me/scratch/ViJdW
 */
void field_run_frame_loop(FieldRenderHalf* render_ctx)
{
    RECT vram_rect;
    FieldRenderHalf* draw_half;
    int controller_state_base;
    u32* field_heap_state;
    s32 is_alt_half;
    FieldRenderHalf* primary_half;
    u32 primitive_cursor;
    char* render_bytes;
    vram_rect.x = 0;
    vram_rect.y = 0;
    vram_rect.w = SCREEN_WIDTH;
    vram_rect.h = VRAM_BACK_DISP_Y + SCREEN_HEIGHT;
    field_heap_state = (u32*)0x801ED000;
    controller_state_base = 0x801ED600;
    ClearImage(&vram_rect, 0, 0, 0);
    draw_half = render_ctx;
    ClearOTagR(draw_half->otag, 0x1010);
    ClearOTagR((draw_half + 1)->otag, 0x1010);
    VSync(0);
    PutDispEnv(&draw_half->disp_env);
    update_controllers();
    SetDispMask(1);
    do
    {
        field_update_scene();
        g_field_current_render_half = (void*)draw_half;
        render_bytes = (char*)draw_half;
        g_field_primitive_cursor = (void*)(render_bytes + 0x40BC);

        ClearOTagR(draw_half->otag, 0x1010);
        if (draw_half == render_ctx)
        {
            primitive_cursor = field_heap_state[0xC / 4];
        }
        else
        {
            primitive_cursor = field_heap_state[0x10 / 4];
        }
        draw_half->primitive_cursor = primitive_cursor;
        field_clear_node_accumulators(g_field_draw_count, g_field_force_two_primitives);
        is_alt_half = (draw_half != render_ctx) ? 1 : 0;
        field_build_frame_commands(draw_half, is_alt_half);
        primary_half = render_ctx;
        if (g_pending_game_state == 0)
        {
            VSync(1);
            field_draw_frame(is_alt_half, draw_half, g_field_draw_count, g_field_force_two_primitives);
            VSync(1);
            DrawSync(0 * 0);
            set_controller_vsync_interval(2);
            VSync(2);
            render_bytes = (char*)render_ctx;
            if (draw_half == primary_half)
            {
                render_bytes = ((char*)draw_half) + 0x7CC4;
            }
            draw_half = (FieldRenderHalf*)render_bytes;
            PutDispEnv(&draw_half->disp_env);
            PutDrawEnv(&draw_half->draw_env);
            field_flush_vram_uploads();
            DrawOTag((u_long*)(((char*)g_field_current_render_half) + 0x403C));
            update_controllers();
            cdrom_process_state();
        }
    } while (g_pending_game_state == 0);
    ((u8*)controller_state_base)[0x13E] = 0;
    ((u8*)controller_state_base)[0x90] = 0 & 0xFFFFFFFF;
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
 * @param render_ctx Field render context (two 0x7CC4-byte frame buffers).
 * @see decomp.me (100%) https://decomp.me/scratch/JAUtV
 */
void field_init_display(FieldRenderHalf* render_ctx)
{
    u_char* render_bytes = (u_char*)render_ctx;
    FieldRenderContextLayout* layout = (FieldRenderContextLayout*)render_bytes;
    RECT vram_rect;

    SetGeomScreen(0x5DC);
    SetGeomOffset(0xA0, 0x78);

    layout->front_display_x = 0;
    layout->front_display_y = 0;
    layout->front_display_width = SCREEN_WIDTH;
    layout->front_display_height = SCREEN_HEIGHT;
    layout->back_display_y = VRAM_BACK_DISP_Y;
    layout->back_display_x = 0;
    layout->back_display_width = SCREEN_WIDTH;
    layout->back_display_height = SCREEN_HEIGHT;

    /* setup RECT for ClearImage */
    vram_rect.x = 0;
    vram_rect.y = 0;
    vram_rect.w = VRAM_WIDTH;
    vram_rect.h = VRAM_HEIGHT;
    ClearImage(&vram_rect, 0, 0, 0);

    SetDefDispEnv(&render_ctx->disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&(render_ctx + 1)->disp_env, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&render_ctx->draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&(render_ctx + 1)->draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    layout->back_draw_dither = 0;
    layout->front_draw_dither = 0;

    field_restore_entry_music();
    g_pending_game_state = 0;
    field_initialize_subsystems(render_ctx);
    g_field_draw_count = 0;
    field_scene_reset(render_ctx);
}

/**
 * @brief Initialize the field text renderer, load its VRAM atlas, and seed the
 *        draw-mode packet at the end of each double-buffered render half.
 * @param render_ctx Field render context containing both render halves.
 * @see decomp.me (100%) https://decomp.me/scratch/v32hU
 */
void field_init_text_renderer(FieldRenderHalf* render_ctx)
{
    s16 font_vram_layout[4];
    FieldDrawModeLayout* draw_modes = (FieldDrawModeLayout*)render_ctx;

    g_text_cursor_y = 0;
    g_text_cursor_x = 0;
    font_vram_layout[0] = 0x120;
    font_vram_layout[1] = 0x1E0;
    font_vram_layout[2] = 0x100;
    font_vram_layout[3] = 0x1FF;
    field_load_vram_resource(0x5DC, font_vram_layout);
    cdrom_wait_queue_empty();
    g_text_clut_base = 0x7FD0;
    draw_modes->front_packet_length = 1;
    draw_modes->front_draw_mode_command = 0xE1000234;
    draw_modes->back_packet_length = 1;
    draw_modes->back_draw_mode_command = 0xE1000234;
}
