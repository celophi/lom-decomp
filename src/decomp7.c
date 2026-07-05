#include "decomp7.h"

/**
 * @brief Run the field overlay's top-level scene loop until a state
 *        transition is requested, then return the next game state.
 * @return Next g_gameState value: the raw g_pending_game_state exit code if it is
 *         a valid state (< 5), otherwise clamped to GAME_STATE_WORLD_MAP.
 * @see decomp.me (100%) https://decomp.me/scratch/CPx5C
 */
s32 run_field_scene(void)
{
    s32 overlay_arg;
    s32 next_state;
    S_801ED480* scene_state = SCENE_STATE;
    overlay_arg = (s32)FUN_80015c28();
    field_init_display(overlay_arg);
    scene_state->unk0 = 0;
    scene_state->unk2 = 0;
    scene_state->unk4 = 0;
    scene_state->unk8 = 0;
    scene_state->unkC = 0;
    do
    {
        next_state = 0x1E;
        D_801158A4 = 0;
        func_8009AFE0(g_scene_mode, g_field_entry_flag, D_8003EC88, g_layout_flag, g_layout_option, g_layout_sub_mode);
        func_80067EB4(0x100, 0x100, 0x100, next_state);
        field_run_frame_loop(overlay_arg);
    } while (g_pending_game_state == 0);
    func_800A379C();
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
    RECT rect;
    FieldRenderHalf* draw_half;
    int new_var2;
    u32* new_var;
    s32 is_alt_half;
    FieldRenderHalf* primary_half;
    u32 temp_v0;
    char* new_var5;
    rect.x = 0;
    rect.y = 0;
    rect.w = SCREEN_WIDTH;
    rect.h = VRAM_BACK_DISP_Y + SCREEN_HEIGHT;
    new_var = (u32*)0x801ED000;
    new_var2 = 0x801ED600;
    ClearImage(&rect, 0, 0, 0);
    draw_half = render_ctx;
    ClearOTagR(draw_half->otag, 0x1010);
    ClearOTagR((draw_half + 1)->otag, 0x1010);
    VSync(0);
    PutDispEnv(&draw_half->disp_env);
    func_800157DC();
    SetDispMask(1);
    do
    {
        func_8009B028();
        D_800473F4 = (void*)draw_half;
        new_var5 = (char*)draw_half;
        D_800473EC = (void*)(new_var5 + 0x40BC);

        ClearOTagR(draw_half->otag, 0x1010);
        if (draw_half == render_ctx)
        {
            temp_v0 = new_var[0xC / 4];
        }
        else
        {
            temp_v0 = new_var[0x10 / 4];
        }
        draw_half->unk40B8 = temp_v0;
        field_clear_node_accumulators(D_800473E8, D_80035248);
        is_alt_half = (draw_half != render_ctx) ? 1 : 0;
        func_800676B4(draw_half, is_alt_half);
        primary_half = render_ctx;
        if (g_pending_game_state == 0)
        {
            VSync(1);
            field_draw_frame(is_alt_half, draw_half, D_800473E8, D_80035248);
            VSync(1);
            DrawSync(0 * 0);
            func_800157B0(2);
            VSync(2);
            new_var5 = (char*)render_ctx;
            if (draw_half == primary_half)
            {
                new_var5 = ((char*)draw_half) + 0x7CC4;
            }
            draw_half = (FieldRenderHalf*)new_var5;
            PutDispEnv(&draw_half->disp_env);
            PutDrawEnv(&draw_half->draw_env);
            func_80056998();
            DrawOTag((u_long*)(((char*)D_800473F4) + 0x403C));
            func_800157DC();
            cdrom_process_state();
        }
    } while (g_pending_game_state == 0);
    ((u8*)new_var2)[0x13E] = 0;
    ((u8*)new_var2)[0x90] = 0 & 0xFFFFFFFF;
    func_800158E0();
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
    u_char* base = (u_char*)render_ctx;
    ArgStruct* arg = (ArgStruct*)base;
    RECT rect;

    SetGeomScreen(0x5DC);
    SetGeomOffset(0xA0, 0x78);

    arg->unk40B0 = 0;
    arg->unk40B2 = 0;
    arg->unk40B4 = SCREEN_WIDTH;
    arg->unk40B6 = SCREEN_HEIGHT;
    arg->unkBD76 = VRAM_BACK_DISP_Y;
    arg->unkBD74 = 0;
    arg->unkBD78 = SCREEN_WIDTH;
    arg->unkBD7A = SCREEN_HEIGHT;

    /* setup RECT for ClearImage */
    rect.x = 0;
    rect.y = 0;
    rect.w = VRAM_WIDTH;
    rect.h = VRAM_HEIGHT;
    ClearImage(&rect, 0, 0, 0);

    SetDefDispEnv(&render_ctx->disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&(render_ctx + 1)->disp_env, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&render_ctx->draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&(render_ctx + 1)->draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    arg->unkBD2E = 0;
    arg->unk406A = 0;

    func_800A3534();
    g_pending_game_state = 0;
    func_800678D4(render_ctx);
    D_800473E8 = 0;
    field_scene_reset(render_ctx);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/v32hU
 */
void func_800160C8(void* arg0)
{
    s16 params[4];
    ArgStruct2* arg = (ArgStruct2*)arg0;

    g_text_cursor_y = 0;
    g_text_cursor_x = 0;
    params[0] = 0x120;
    params[1] = 0x1E0;
    params[2] = 0x100;
    params[3] = 0x1FF;
    func_80086310(0x5DC, params);
    cdrom_wait_queue_empty();
    g_text_atlas_base = 0x7FD0;
    arg->unk7CBF = 1;
    arg->unk7CC0 = 0xE1000234;
    arg->unkF983 = 1;
    arg->unkF984 = 0xE1000234;
}