#include "decomp3.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/hDNyF
 */
s32 FUN_80021fbc(void)
{
    func_80023AD0();
    return 0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/z7ZEh
 */
s32 func_80021FDC(void)
{
    func_80023BE0();
    return 0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/0q180
 */
s32 func_80021FFC(s32 arg0)
{
    s32 temp_v0;

    temp_v0 = func_800235F8();
    if (temp_v0 == 0)
    {
        func_80023BB8(arg0 + 0x10);
    }
    return temp_v0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/iVOOb
 */
void func_80022040(s32 arg0)
{
    D_8004D430[0] = arg0;
    func_80028E84(0x10);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/9M4hF
 */
void func_80022068(s32 arg0)
{
    D_8004D430[0] = arg0;
    func_80028E84(0x11);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/4GVez
 */
void func_80022090(void)
{
    func_80028E84(0x40);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/c2C3m
 */
void func_800220B0(s32 arg0, s32 arg1)
{
    D_8004D430[0] = arg0;
    D_8004D430[1] = arg1;
    D_8004D430[2] = 0;
    func_80028E84(0x14);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/d6xXt
 */
s32 func_800220E4(s32 arg0, s32 arg1)
{
    s32 temp_v0;

    D_8004D430[0] = arg0;
    temp_v0 = func_80028E84(0x19);
    D_8004D430[0] = (s32)(arg1 & 0x7F);
    D_8004D430[3] = 0;
    func_80028E84(0xC0);
    return temp_v0;
}