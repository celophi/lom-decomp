#include "common.h"

extern u8 D_80122C1C;
extern u8 g_menuLayoutBuffer[];
extern void func_800B2844(s32 arg0, u8 *arg1, u8 arg2);

void func_800C9CE4(void)
{
    u8 *base;
    u8 *rec;
    s32 idx;

    idx = ((u8 *)&D_80122C1C)[D_80122C1C + 1] * 0x60;
    base = g_menuLayoutBuffer;
    rec = idx + base;
    D_80122C1C = rec[0x2F3C];
    base = base + 0x2EF4;
    func_800B2844(3, idx + base, 0xFF);
}
