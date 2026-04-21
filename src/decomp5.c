#include "decomp5.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/CJTY6
 */
void func_800235A8(s32* arg0, s32* arg1, s32 arg2, s32 arg3)
{
    s32* t0 = arg1 + 3;
    int new_var;
    s32* v1 = arg0 + 3;
    s32 new_var2;
    char new_var3;
    do
    {
        *arg1 = (*arg0) + arg2;
        new_var2 = v1[-2];
        arg0 += 4;
        arg3 -= 1;
        t0[-2] = new_var2 + arg2;
        (new_var2 = 4);
        arg1 += new_var2;
        new_var = -1;
        t0[new_var] = v1[new_var];
        *t0 = *v1;
        v1 += 4;
        t0 += 4;
    } while (arg3 != 0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/scY8u
 */
s32 func_800235F8(s32* arg0)
{
    return *arg0 + 0xB0BEB4BF;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/qI6jZ
 */
void func_8002360C(void)
{
    func_80024230(0);
    D_8003EC4C = 0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/oy7T9
 */
void func_80023630(void)
{
    D_8003EC4C = 1;
    func_80024230(&func_8002360C);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/D2YiT
 */
void func_80023660(s32 arg0, s32 arg1)
{
    D_8003EC4C = 1;
    func_80024230(&func_8002360C);
    func_800241A0(arg0, arg1);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/lLOqn
 */
void func_800236B0(s32 arg0, s32 arg1)
{
    func_80023630();
    func_80024140(arg0, arg1);
}
