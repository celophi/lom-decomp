#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern s16 D_80122C10;

void func_800C62E8(void)
{
    s32 i;
    s32 count;
    u8 *p;
    u16 v0;
    s16 *ptr;

    count = 0;
    for (i = 0; i < 0x64; i++)
    {
        p = &g_menuLayoutBuffer[i * 0x40];
        if (p[0xCE0] == 0)
        {
            count++;
        }
    }
    ptr = &D_80122C10;
    if (count >= *ptr)
    {
        v0 = 0;
    }
    else
    {
        v0 = (u16) *ptr - count;
    }
    *ptr = v0;
}
