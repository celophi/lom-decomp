#include "common.h"

extern u8 D_80117E98[][0x10];

void func_800A2958(s32 arg0)
{
    s32 i;

    for (i = 0; i < 0xF; i++)
    {
        D_80117E98[arg0][i] = D_80117E98[arg0][i + 1];
    }
}
