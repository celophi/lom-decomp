#include "common.h"

extern s32 D_80117E88[];
extern s32 D_80117EB8[];
extern s16 D_80117E90[];

void func_800A255C(void)
{
    D_80117E88[1] = 0;
    D_80117E88[0] = 0;
    D_80117EB8[1] = 0x10;
    D_80117EB8[0] = 0x10;
    D_80117E90[1] = 0;
    D_80117E90[0] = 0;
}
