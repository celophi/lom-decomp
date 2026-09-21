#include "common.h"

extern s32 D_801B2E10;
extern s32 D_801B2E14;
extern void func_8006B328(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g,
                          s32 h, s32 i, s32 j, s32 k, s32 l, s32 m, s32 n,
                          s32 o, s32 p, s32 q);

/** @brief World-map step: spawn an effect object then count down a timer. */
void func_800A4FC8(void)
{
    func_8006B328(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32,
                  0x64, 1, 0x7F, 0x7F, 0, 0);
    if (--D_801B2E14 == 0)
    {
        D_801B2E10 += 1;
    }
}
