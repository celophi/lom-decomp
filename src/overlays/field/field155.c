#include "common.h"

extern u16 D_800F0E98[];

extern void func_800B2844(s32, void *, s32);

void func_800C2228(s32 idx)
{
    func_800B2844(0, (u8 *)D_800F0E98 + D_800F0E98[idx], 0x15);
}
