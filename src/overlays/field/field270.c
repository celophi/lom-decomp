#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern u8 D_80043CB8[];

extern u8 *func_800A9060(void);
extern void func_800A8F8C(u8 *dst, u8 *src);

void func_800C6A90(void)
{
    while (func_800A9060() != 0)
    {
        func_800A8F8C(func_800A9060(), D_80043CB8);
    }

    g_menuLayoutBuffer[0x29D5] += 9;
}
