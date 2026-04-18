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

/**
 * decomp.me link (100%) https://decomp.me/scratch/jigab
 */
void func_8002213C(s32 arg0, s32 arg1)
{
    D_8004D430[0] = arg0;
    D_8004D430[1] = arg1;
    func_80028E84(0x12);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/9AZZL
 */
void func_8002216C(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_a1;
    s32 temp_a2;
    s32 temp_a3;

    D_8004D430[0] = (s32)(arg0 & 0x3FF);
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a2 = arg2 & 0xFF;
    temp_a3 = arg3 & 0x7F;
    D_8004D430[1] = temp_a1;
    D_8004D430[2] = temp_a2;
    D_8004D430[3] = temp_a3;
    func_80028E84(0x20);
};

/**
 * decomp.me link (96.97%) https://decomp.me/scratch/4tVNg
 */
s32 func_800221BC(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    long long new_var2;
    s32 var_v0;
    unsigned char new_var;
    var_v0 = func_800235F8();
    if (var_v0 == 0)
    {
        D_8004D430[0] = arg0;
        new_var = arg2;
        D_8004D430[1] = (s32)(arg1 & 0xFFFFFF);
        D_8004D430[2] = (s32)(new_var & 0xFF);
        D_8004D430[3] = (s32)(arg3 & 0x7F);
        func_80028E84(0x24);
        new_var2 = arg0;
        var_v0 = new_var2;
    }
    return var_v0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/lu9nS
 */
void func_80022240(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    D_8004D430[0] = arg0;
    temp_a1 = arg1 & 0xFFFFFF;
    D_8004D430[1] = temp_a1;
    func_80028E84(0x21);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/0mLzI
 */
void func_8002227C(s32 arg0)
{
    D_8004D430[0] = arg0 & 0x3FF;
    func_80028E84(0x30);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/yZloM
 */
s32 func_800222A8(void)
{
    s32 bits;
    unsigned char* ptr;
    s32 new_var;
    s32 acc;
    unsigned int mask;
    new_var = D_8004D400;
    bits = new_var;
    acc = bits == 0;
    if (acc)
    {
        return 0;
    }
    ptr = D_8004B430;
    acc = 0;
    mask = 0x1000;
    do
    {
        if (bits & mask)
        {
            acc |= *((s32*)(ptr + 0x28));
        }
        mask <<= 1;
        ptr += 0x118;
    } while (mask & 0xFFFFFF);
    return acc & 0xFFFFFF;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/OvqYq
 */
s32 func_80022310(s32 arg0)
{
    s32 new_var;
    s32 bits;
    unsigned char* ptr;
    unsigned int mask;
    if (arg0 == 0)
    {
        return 0;
    }
    bits = D_8004D400;
    if (bits == 0)
    {
        return 0;
    }
    ptr = D_8004B430;
    mask = 0x1000;
    do
    {
        if ((new_var = bits) & mask)
        {
            if (arg0 == (*((s32*)(ptr + 0x28))))
            {
                return 1;
            }
        }
        mask <<= 1;
        ptr += 0x118;
    } while (mask & 0xFFFFFF);
    return 0;
}