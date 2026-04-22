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