#include "common.h"

typedef struct
{
    u8 pad0[0x29D6];
    u8 index;
    u8 pad29D7[5];
    u32 words[1];
} MenuLayoutBuffer;

extern MenuLayoutBuffer g_menuLayoutBuffer;
extern s16 D_80122C1C;
extern u8 D_800459AE;

void func_800C6C80(void)
{
    s16 *p = &D_80122C1C;
    s32 arg0 = p[0];
    s32 arg1 = p[-1];
    s32 arg2 = p[-2];

    if (arg0 == 0xFF)
    {
        D_800459AE = 0;
        return;
    }

    if (g_menuLayoutBuffer.index < 0x28)
    {
        g_menuLayoutBuffer.words[g_menuLayoutBuffer.index] =
            (g_menuLayoutBuffer.words[g_menuLayoutBuffer.index] & ~0xFC) | ((arg0 & 0x3F) << 2);
        g_menuLayoutBuffer.words[g_menuLayoutBuffer.index] =
            (g_menuLayoutBuffer.words[g_menuLayoutBuffer.index] & ~0xF00) | ((arg1 & 0xF) << 8);
        g_menuLayoutBuffer.words[g_menuLayoutBuffer.index] =
            (g_menuLayoutBuffer.words[g_menuLayoutBuffer.index] & ~0xF000) | ((arg2 & 0xF) << 12);
        g_menuLayoutBuffer.words[g_menuLayoutBuffer.index] |= 3;
        g_menuLayoutBuffer.words[g_menuLayoutBuffer.index] &= ~0x10000;
        g_menuLayoutBuffer.index++;
    }
}
