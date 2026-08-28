#include "common.h"

extern s32 D_80165520;
extern s32 D_80165490[];

void func_801449F0(void)
{
    s32 i;
    s32 val;

    D_80165520 = 0x28;
    val = -1;
    for (i = 14; i >= 0; i--)
    {
        D_80165490[i] = val;
    }
}
