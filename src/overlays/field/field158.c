#include "common.h"

extern u8 D_801148B0[];

void *func_800C2928(s32 arg0, s32 arg1)
{
    u8 *base;
    s32 offset;
    s16 *table;

    base = D_801148B0 + (arg0 << 0xC);
    offset = *(s32 *)(base + 4);
    table = (s16 *)(base + offset);
    return (u8 *)table + table[arg1 & 0xFFFF];
}
