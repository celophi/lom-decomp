#include "common.h"

extern void func_8006B328(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_801B2E24;
extern s32 D_801B2E20;

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800A5504(void)
{
    func_8006B328(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 1);
    if (--D_801B2E24 == 0)
    {
        D_801B2E20 += 1;
    }
}
