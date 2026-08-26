#include "common.h"

extern u8 g_menuLayoutBuffer[];

void func_800CA1A0(s32 arg0)
{
    u8 *rec;

    rec = &g_menuLayoutBuffer[arg0 * 0xC];
    g_menuLayoutBuffer[0x2E4]++;
    rec[0x2F0] |= 1;
    rec[0x2F3] = g_menuLayoutBuffer[0x2E4];
}
