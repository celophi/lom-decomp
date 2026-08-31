#include "common.h"

extern s32 D_80165FF0;
extern s32 D_801227C4;
extern u8 *D_8012271C;

void func_80146694(void)
{
    u8 **base;
    s32 index;
    s32 offset;

    func_800141EC(0x5E2, D_80165FF0);
    func_80013F2C();

    base = &D_8012271C;
    index = D_801227C4;
    offset = index * 0x60 + 0x2EF4;
    func_80016764(D_80165FF0, *base + offset);
}
