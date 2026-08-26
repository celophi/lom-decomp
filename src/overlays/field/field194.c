#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern u8 D_80122C1F;

void func_800C9448(void)
{
    s32 i;
    s32 count;
    u8 *p;

    count = 0;
    for (i = 0; i < 4; i++)
    {
        p = &g_menuLayoutBuffer[i * 0x40];
        if (p[0x3160] == 0)
        {
            count++;
        }
    }
    D_80122C1F = (u8) count;
}
