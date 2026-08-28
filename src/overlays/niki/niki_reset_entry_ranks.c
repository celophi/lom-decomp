#include "common.h"

extern s32 D_80164EB0;
extern s32 D_80164E20[];

void func_80144BC0(void)
{
    s32 i;
    s32 val;

    D_80164EB0 = 0x28;
    val = -1;
    for (i = 14; i >= 0; i--)
    {
        D_80164E20[i] = val;
    }
}
