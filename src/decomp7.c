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
    func_80015F88(overlay_arg);
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
 * @param arg0 Field render context (two 0x7CC4-byte frame buffers).
 * @see decomp.me (100%) https://decomp.me/scratch/ViJdW
 */
void field_run_frame_loop(void* arg0)
{
    RECT rect;
    FieldRenderHalf* draw_half;
    int new_var4;
    int new_var2;
    u32* new_var;
    s32 is_alt_half;
    FieldRenderHalf* primary_half;
    u32 temp_v0;
    char* new_var5;
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x140;
    rect.h = 0x1D8;
    new_var = (u32*)0x801ED000;
    new_var2 = 0x801ED600;
    ClearImage(&rect, 0, 0, 0);
    draw_half = (FieldRenderHalf*)arg0;
    ClearOTagR(draw_half->otag, 0x1010);
    ClearOTagR((draw_half + 1)->otag, 0x1010);
    VSync(0);
    PutDispEnv(&draw_half->disp_env);
    func_800157DC();
    SetDispMask(new_var4 = 1);
    do
    {
        func_8009B028();
        D_800473F4 = (void*)draw_half;
        new_var5 = (char*)draw_half;
        D_800473EC = (void*)(new_var5 + 0x40BC);

        ClearOTagR(draw_half->otag, 0x1010);
        if (draw_half == ((FieldRenderHalf*)arg0))
        {
            temp_v0 = new_var[0xC / 4];
        }
        else
        {
            temp_v0 = new_var[0x10 / 4];
        }
        draw_half->unk40B8 = temp_v0;
        field_clear_node_accumulators(D_800473E8, D_80035248);
        is_alt_half = (draw_half != ((FieldRenderHalf*)arg0)) ? (1) : (0);
        func_800676B4(draw_half, is_alt_half);
        primary_half = (FieldRenderHalf*)arg0;
        if (g_pending_game_state == 0)
        {
            VSync(1);
            field_draw_frame(is_alt_half, draw_half, D_800473E8, D_80035248);
            VSync(new_var4);
            DrawSync(0 * 0);
            func_800157B0(2);
            VSync(2);
            new_var5 = (char*)arg0; // FIX 2: temp = arg0 (move v0,s2)
            if (draw_half == primary_half) // FIX 2: bne skips this block
            {
                new_var5 = ((char*)draw_half) + 0x7CC4; // addiu v0,s0,0x7cc4
            }
            draw_half = (FieldRenderHalf*)new_var5; // FIX 2: always: move s0,v0
            PutDispEnv(&draw_half->disp_env);
            PutDrawEnv(&draw_half->draw_env);
            func_80056998();
            DrawOTag((u_long*)(((char*)D_800473F4) + 0x403C));
            func_800157DC();
            cdrom_process_state();
        }
    } while (g_pending_game_state == 0);
    ((u8*)new_var2)[0x13E] = 0;             // FIX 3: sb [0x13e] standalone
    ((u8*)new_var2)[0x90] = 0 & 0xFFFFFFFF; // FIX 3: moved before func_800158E0
    func_800158E0();                        // FIX 3: [0x90] becomes delay slot
    akao_cmd_f0();
    akao_cmd_f1();
    DrawSync(0);
    VSync(0);
    SetDispMask(0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/JAUtV
 */
void func_80015F88(void* arg0)
{
    u_char* base = (u_char*)arg0;      /* for pointer arithmetic */
    ArgStruct* arg = (ArgStruct*)base; /* for field access */
    RECT rect;                         /* replaced separate s16 variables */

    SetGeomScreen(0x5DC);
    SetGeomOffset(0xA0, 0x78);

    arg->unk40B0 = 0;
    arg->unk40B2 = 0;
    arg->unk40B4 = 0x140;
    arg->unk40B6 = 0xF0;
    arg->unkBD76 = 0xE8;
    arg->unkBD74 = 0;
    arg->unkBD78 = 0x140;
    arg->unkBD7A = 0xF0;

    /* setup RECT for ClearImage */
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x400;
    rect.h = 0x200;
    ClearImage(&rect, 0, 0, 0);

    SetDefDispEnv((DISPENV*)(base + 0x4040), 0, 0, 0x140, 0xF0);
    SetDefDispEnv((DISPENV*)(base + 0xBD04), 0, 0xE8, 0x140, 0xF0);
    SetDefDrawEnv((DRAWENV*)(base + 0x4054), 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv((DRAWENV*)(base + 0xBD18), 0, 8, 0x140, 0xE0);

    arg->unkBD2E = 0;
    arg->unk406A = 0;

    func_800A3534();
    g_pending_game_state = 0;
    func_800678D4(arg0);
    D_800473E8 = 0;
    field_scene_reset(arg0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/v32hU
 */
void func_800160C8(void* arg0)
{
    s16 params[4]; // replaces sp10, sp12, sp14, sp16
    ArgStruct2* arg = (ArgStruct2*)arg0;

    D_80047408 = 0;
    D_80047404 = 0;
    params[0] = 0x120;
    params[1] = 0x1E0;
    params[2] = 0x100;
    params[3] = 0x1FF;
    func_80086310(0x5DC, params); // passes pointer to the array
    cdrom_wait_queue_empty();
    D_80047400 = 0x7FD0;
    arg->unk7CBF = 1;
    arg->unk7CC0 = 0xE1000234;
    arg->unkF983 = 1;
    arg->unkF984 = 0xE1000234;
}