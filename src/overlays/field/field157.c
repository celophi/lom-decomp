#include "common.h"

extern u8 D_801148B0[];

s32 func_800C28F8(s32 arg0, u16 arg1)
{
    u8 *base = D_801148B0 + (arg0 << 12);
    u8 *table = base + *(s32 *)base;

    return (s32)table + *(s16 *)(arg1 * 2 + table);
}
