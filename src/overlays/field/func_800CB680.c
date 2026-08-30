#include "common.h"

typedef struct
{
    u8 pad0[0x29D6];
    u8 index;
    u8 pad29D7[5];
    u32 words[1];
} MenuLayoutBuffer;

extern MenuLayoutBuffer g_menuLayoutBuffer;

void func_800CB680(u32 arg0, u32 arg1, u32 arg2)
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
