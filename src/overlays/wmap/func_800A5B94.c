#include "common.h"

extern void func_8006B328(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_801B2E34;
extern s32 D_801B2E30;

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800A5B94(void)
{
    func_8006B328(0xD2, 0xE6, 2, -1, -1, -8, 0x28, 8, -0xB4, 0x168, -0xB4, 0x168, 1, 1, 0x81, 4, 3);
    if (--D_801B2E34 == 0)
    {
        D_801B2E30 += 1;
    }
}
