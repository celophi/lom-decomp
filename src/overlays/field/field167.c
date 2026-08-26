#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern s8 D_800459AF;

extern void func_800C3BB0(void);
extern void func_800C3F18(s32 arg0, void *arg1);

void func_800C3B50(s32 arg0)
{
    if (arg0 == 3)
    {
        g_menuLayoutBuffer[0x29D7] = g_menuLayoutBuffer[0x29D4] >> 4;
    }
    else
    {
        D_800459AF = arg0;
    }

    func_800C3BB0();
    func_800C3F18((s8) g_menuLayoutBuffer[0x29D7], &g_menuLayoutBuffer[0xA90]);
}

void func_800C3BB0(void)
{
    func_800C3BD8(D_800459AF);
}
