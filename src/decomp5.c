#include "common.h"

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