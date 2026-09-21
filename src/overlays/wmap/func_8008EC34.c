#include "common.h"

extern void func_8006B328(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6,
                          s32 a7, s32 a8, s32 a9, s32 a10, s32 a11, s32 a12,
                          s32 a13, s32 a14, s32 a15, s32 a16);
extern s32 D_801B2A64;
extern s32 D_801B2A60;

/** @brief World-map step handler: spawn a sprite via a long argument list, then countdown-advance. */
void func_8008EC34(void)
{
    func_8006B328(0x64, 0xC8, 1, -1, -2, -2, 0, 0xF, -0xAF, 0x15E, -0xAF, 0x15E,
                  0x32, 1, 0x81, 2, 0);
    if (--D_801B2A64 == 0)
    {
        D_801B2A60 += 1;
    }
}
