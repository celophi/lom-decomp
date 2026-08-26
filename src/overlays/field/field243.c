#include "common.h"

extern s32 D_801227F8[];
extern s32 D_80122908;
extern u8 D_80122910[];

void func_800A8D8C(s32 arg0, u8 arg1)
{
    s32 index = D_80122908;

    if (index < 10)
    {
        D_801227F8[index] = arg0;
        D_80122910[index] = arg1;
        D_80122908++;
    }
}
