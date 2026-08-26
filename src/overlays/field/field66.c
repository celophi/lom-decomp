#include "common.h"

extern s32 D_801226A0[];

void func_800A6204(void)
{
    D_801226A0[5] &= 0xFF81FFFF;
    D_801226A0[3] &= 0xFF81FFFF;
    D_801226A0[1] &= 0xFF81FFFF;
}
