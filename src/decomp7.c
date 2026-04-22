#include "decomp7.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/CPx5C
 */
s32 FUN_80015c58(void)
{
    s32 temp_v0;
    s32 result;
    u8* base = (u8*)0x801ED480;
    temp_v0 = (s32)FUN_80015c28();
    func_80015F88(temp_v0);
    *((u16*)(base + 0)) = 0;
    *((u16*)(base + 2)) = 0;
    *((u32*)(base + 4)) = 0;
    *((u32*)(base + 8)) = 0;
    *((u32*)(base + 12)) = 0;
    do
    {
        result = 0x1E;
        D_801158A4 = 0;
        func_8009AFE0(D_8003EC90, D_80042FCC, D_8003EC88, D_80042FC4, D_8003EC94, D_80046FD8);
        func_80067EB4(0x100, 0x100, 0x100, result);
        func_80015D6C(temp_v0);
    } while (D_8010D018 == 0);
    func_800A379C();
    FUN_80022aa8();
    FUN_80022ac8();
    result = D_8010D018;
    if (result < 5)
    {
        return result;
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
        func_800520A0(D_800473E8, D_80035248);
        s1 = (var_s0 != ((ObjStruct*)arg0)) ? (1) : (0);
        func_800676B4(var_s0, s1);
        new_var3 = (ObjStruct*)arg0;
        if (D_8010D018 == 0)
        {
            VSync(1);
            func_80051FF8(s1, var_s0, D_800473E8, D_80035248);
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
            CD_UpdateAndProcessQueue();
        }
    } while (D_8010D018 == 0);
    ((u8*)new_var2)[0x13E] = 0;             // FIX 3: sb [0x13e] standalone
    ((u8*)new_var2)[0x90] = 0 & 0xFFFFFFFF; // FIX 3: moved before func_800158E0
    func_800158E0();                        // FIX 3: [0x90] becomes delay slot
    FUN_80022aa8();
    FUN_80022ac8();
    DrawSync(0);
    VSync(0);
    SetDispMask(0);
}