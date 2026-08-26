#include "common.h"

extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 g_menuLayoutBuffer[];
extern u32 D_80122C00;

void func_800C7628(void)
{
    u8 *base;
    s32 idx;
    u8 *p;

    if (g_gosub_result_count != 0)
    {
        idx = g_gosub_result_values[0];
        base = g_menuLayoutBuffer;
        p = &base[idx * 0x60];
        D_80122C00 = *(u16 *) (p + 0x2F4E);
    }
}
