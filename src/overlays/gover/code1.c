
#include "gover.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/1qYnn
 */
void func_80140004(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    RECT rect;
    u8* base;
    u8* new_var;
    u16* second;
    u8(*new_var2)[];
    new_var2 = &D_801407A0;
    VSync(0);
    DrawSync(0);
    base = *new_var2;
    new_var = base + 0x49C;
    *((u16*)(base + 0)) = 0;
    *((u16*)(base + 2)) = 0;
    *((u16*)(base + 4)) = 0x140;
    *((u16*)(base + 6)) = 0xF0;
    second = (u16*)new_var;
    second[0] = 0;
    second[1] = 0xE8;
    second[2] = 0x140;
    second[3] = 0xF0;
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x400;
    rect.h = 0x200;
    ClearImage(&rect, 0, 0, 0);
    SetDefDispEnv((DISPENV*)(base - 0x70), 0, 0, 0x140, 0xF0);
    SetDefDispEnv((DISPENV*)(base + 0x42C), 0, 0xE8, 0x140, 0xF0);
    SetDefDrawEnv((DRAWENV*)(base - 0x5C), 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv((DRAWENV*)(base + 0x440), 0, 8, 0x140, 0xE0);
    base = base - 0x90;
    *((u8*)(base + 0x4E6)) = 0;
    *((u8*)(base + 0x4A)) = 0;
    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0x1E0;
    base = base - 0x90;
    func_80140538(arg1 + 0xFFC, (s16*)(&rect), arg0);
    FUN_80022aa8();
    FUN_80022ac8();
    func_800224D8(0x7F);
    if (arg3 != (-1))
    {
        func_80140648(arg3);
        func_800A39A8(0, 0x80, 0, 0);
    }
    if (arg2 != (-1))
    {
        func_800A368C(arg2, 0);
        D_8011588C = 0x7F;
        func_800A380C();
        FUN_8002279c(0, 0x7F);
    }
    D_80141048 = 4;
    D_80140708 = 4;
    func_801401F0();
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/LOxbx
 */
void func_801401F0(void)
{
    u_char* var_s0;
    u_char* var_s1;
    int new_var;
    u_char* var_v0;
    u8 dummy[8];
    u_long* p_d40708;
    func_800AA02C();
    var_s0 = (u_char*)D_80140710;
    ClearOTagR((u_long*)var_s0, 8);
    ClearOTagR((u_long*)(var_s0 - (-0x49C)), 8);
    VSync(0);
    PutDispEnv((DISPENV*)(var_s0 + 0x20));
    func_800157DC();
    SetDispMask(1);
    {
        var_s1 = var_s0;
        while (1)
        {
            var_s1 = var_s0;
            ClearOTagR((u_long*)var_s1, 8);
            *((void**)(var_s1 + 0x498)) = (void*)(var_s1 + 0x98);
            func_800A9E78();
            func_80140380((s32*)var_s1);
            DrawSync(0);
            func_800157B0(2);
            if (!D_80141048)
            {
            }
            VSync(2);
            p_d40708 = &D_80140708;
            if ((D_80141048 == 0x80) && (D_80122988 & 0x260))
            {
                func_800227D0(0, 0x20, 0);
                *p_d40708 = -4;
            }
            if (D_80141048 == (0 & 0xFF))
            {
                break;
            }
            var_v0 = (u_char*)D_80140710;
            if (var_s0 == ((u_char*)D_80140710))
            {
                var_v0 = var_s0 + 0x49C;
            }
            var_s0 = var_v0;
            PutDispEnv((DISPENV*)(var_s0 + 0x20));
            new_var = 0x1C;
            PutDrawEnv((DRAWENV*)(var_s0 + 0x34));
            DrawOTag((u_long*)(var_s1 + new_var));
            func_800157DC();
            CD_UpdateAndProcessQueue();
        }
    }
    DrawSync(0);
    VSync(0);
    func_800158E0();
    FUN_80022aa8();
    FUN_80022ac8();
    SetDispMask(0);
    D_8003EC90 = 0;
    func_800AA02C();
    D_8010D018 = 1;
}