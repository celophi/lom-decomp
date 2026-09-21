#include "common.h"

extern void func_8006B328(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_801B2B60;
extern s32 D_801B2B64;

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_80095000(void)
{
    func_8006B328(0x64, 0x6C, 4, -1, -1, -4, 0, 8, -0x6E, 0xF0, -0x6E, 0xF0, 1, 1, 0x7F, 4, 0);
    if (--D_801B2B64 == 0)
    {
        D_801B2B60 += 1;
    }
}
