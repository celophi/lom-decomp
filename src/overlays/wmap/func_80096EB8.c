#include "common.h"

extern void func_8006B328(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_801B2BC4;
extern s32 D_801B2BC0;

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_80096EB8(void)
{
    func_8006B328(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0);
    if (--D_801B2BC4 == 0)
    {
        D_801B2BC0 += 1;
    }
}
