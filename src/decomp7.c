#include "decomp7.h"

/**
 * @brief Run the field overlay's top-level scene loop until a state
 *        transition is requested, then return the next game state.
 * @return Next g_gameState value: the raw D_8010D018 exit code if it is
 *         a valid state (< 5), otherwise clamped to GAME_STATE_WORLD_MAP.
 * @see decomp.me (100%) https://decomp.me/scratch/CPx5C
 */
s32 run_field_scene(void)
{
    s32 overlay_arg;
    s32 next_state;
    u8* base = (u8*)0x801ED480;
    overlay_arg = (s32)FUN_80015c28();
    func_80015F88(overlay_arg);
    *((u16*)(base + 0)) = 0;
    *((u16*)(base + 2)) = 0;
    *((u32*)(base + 4)) = 0;
    *((u32*)(base + 8)) = 0;
    *((u32*)(base + 12)) = 0;
    do
    {
        next_state = 0x1E;
        D_801158A4 = 0;
        func_8009AFE0(g_scene_mode, g_field_entry_flag, D_8003EC88, g_layout_flag, g_layout_option, g_layout_sub_mode);
        func_80067EB4(0x100, 0x100, 0x100, next_state);
        func_80015D6C(overlay_arg);
    } while (D_8010D018 == 0);
    func_800A379C();
    akao_cmd_f0();
    akao_cmd_f1();
    next_state = D_8010D018;
    if (next_state < 5)
    {
        return next_state;
    }
    return 1;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/ViJdW
 */
void func_80015D6C(void* arg0)
{
    RECT rect;
    ObjStruct* var_s0;
    int new_var4;
    int new_var2;
    u32* new_var;
    s32 s1;
    ObjStruct* new_var3;
    u32 temp_v0;
    char* new_var5;
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x140;
    rect.h = 0x1D8;
    new_var = (u32*)0x801ED000; // FIX 1: new_var (s3) initialized first
    new_var2 = 0x801ED600;      // FIX 1: new_var2 (s8) initialized second
    ClearImage(&rect, 0, 0, 0);
    var_s0 = (ObjStruct*)arg0;
    ClearOTagR((u_long*)var_s0, 0x1010);
    ClearOTagR((u_long*)(((char*)var_s0) + 0x7CC4), 0x1010);
    VSync(0);
    PutDispEnv((DISPENV*)(((char*)var_s0) + 0x4040));
    func_800157DC();
    SetDispMask(new_var4 = 1);
    do
    {
        func_8009B028();
        D_800473F4 = (void*)var_s0;
        new_var5 = (char*)var_s0;
        D_800473EC = (void*)(new_var5 + 0x40BC);
        if (1)
        {
        }
        ClearOTagR((u_long*)var_s0, 0x1010);
        if (var_s0 == ((ObjStruct*)arg0))
        {
            temp_v0 = new_var[0xC / 4];
            if (((!arg0) && (!arg0)) && (!arg0))
            {
            }
        }
        else
        {
            temp_v0 = new_var[0x10 / 4];
        }
        var_s0->unk40B8 = temp_v0;
        field_clear_node_accumulators(D_800473E8, D_80035248);
        s1 = (var_s0 != ((ObjStruct*)arg0)) ? (1) : (0);
        func_800676B4(var_s0, s1);
        new_var3 = (ObjStruct*)arg0;
        if (D_8010D018 == 0)
        {
            VSync(1);
            field_draw_frame(s1, var_s0, D_800473E8, D_80035248);
            VSync(new_var4);
            DrawSync(0 * 0);
            func_800157B0(2);
            VSync(2);
            new_var5 = (char*)arg0; // FIX 2: temp = arg0 (move v0,s2)
            if (var_s0 == new_var3) // FIX 2: bne skips this block
            {
                new_var5 = ((char*)var_s0) + 0x7CC4; // addiu v0,s0,0x7cc4
            }
            var_s0 = (ObjStruct*)new_var5; // FIX 2: always: move s0,v0
            PutDispEnv((DISPENV*)(((char*)var_s0) + 0x4040));
            PutDrawEnv((DRAWENV*)(((char*)var_s0) + 0x4054));
            func_80056998();
            DrawOTag((u_long*)(((char*)D_800473F4) + 0x403C));
            func_800157DC();
            cdrom_process_state();
        }
    } while (D_8010D018 == 0);
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
    D_8010D018 = 0;
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