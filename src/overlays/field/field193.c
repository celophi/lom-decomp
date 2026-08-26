#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern u8 D_80122C00;
extern void func_800B2844(s32 arg0, u8 *arg1, u8 arg2);

void func_800C9404(void)
{
    D_80122C00 = g_menuLayoutBuffer[0x840];
    if (D_80122C00 != 0)
    {
        func_800B2844(0, &g_menuLayoutBuffer[0x840], 0xFF);
    }
}
