#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern u16 D_80122C16;

void func_800C9D44(void)
{
    s32 i;
    s32 count;
    u8 *p;

    count = 0;
    for (i = 0; i < 5; i++)
    {
        p = &g_menuLayoutBuffer[i * 0x60];
        if (p[0x2EF4] != 0)
        {
            count++;
        }
    }
    D_80122C16 = (u16) count;
}
