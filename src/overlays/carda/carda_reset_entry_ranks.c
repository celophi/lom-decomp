#include "common.h"

extern s32 D_80166438;
extern s32 D_801663A8[];

void func_80147C5C(void)
{
    s32 i;
    s32 val;

    D_80166438 = 0x28;
    val = -1;
    for (i = 16; i >= 0; i--)
    {
        D_801663A8[i] = val;
    }
}
