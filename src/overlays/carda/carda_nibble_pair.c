#include "common.h"
extern u16 D_80165F14[];
void func_8014A87C(s32 arg0,s32 arg1,s32 arg2,s32 arg3,s32 arg4,s32 arg5)
{
    u16 pair[3];
    s32 row;
    s32 adjusted;
    s32 off;
    u16 *base;

    adjusted = arg2;
    if (arg2 < 0)
        adjusted = arg2 + 15;
    row = adjusted >> 4;
    off = row * 2;
    base = D_80165F14;
    pair[0] = *(u16 *)((u8 *)base + off);
    off = (arg2 - row * 16) * 2;
    pair[1] = *(u16 *)((u8 *)base + off);
    pair[2] = 0;
    func_8014A900(arg0, arg1, pair, arg3, arg4, 0, arg5);
}
