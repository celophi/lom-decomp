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

extern u8 g_menuLayoutBuffer[];
extern u8 D_80122C1F;

void func_800C9488(void)
{
    s32 i;
    s32 count;
    u8 *p;

    count = 0;
    for (i = 0; i < 0x64; i++)
    {
        p = &g_menuLayoutBuffer[i * 0x40];
        if (p[0xCE0] == 0)
        {
            count++;
        }
    }
    D_80122C1F = (u8) count;
}

extern u8 D_80122A08[];

void func_800C94C8(void)
{
    D_80122A08[0] = 0;
    D_80122A08[0x40] = 0;
    D_80122A08[0x80] = 0;
    D_80122A08[0xC0] = 0;
    *(s32 *)&D_80122A08[0x34] = 0;
    *(s32 *)&D_80122A08[0x74] = 0;
    *(s32 *)&D_80122A08[0xB4] = 0;
    *(s32 *)&D_80122A08[0xF4] = 0;
}
