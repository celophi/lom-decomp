#include "common.h"

extern u8* D_80122B74;

void func_800C299C(s32 arg0)
{
    u8* p = D_80122B74;
    u32 idx = (u32)arg0 >> 5;
    *(s32*)(p + (idx << 2) + 0x30D4) |= 1 << (arg0 & 0x1F);
}
