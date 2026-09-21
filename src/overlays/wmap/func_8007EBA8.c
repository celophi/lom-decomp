#include "common.h"

extern s32 D_801B2778;
extern s32 D_801B277C;
extern void func_8006B328(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g,
                          s32 h, s32 i, s32 j, s32 k, s32 l, s32 m, s32 n,
                          s32 o, s32 p, s32 q);

/** @brief World-map step: spawn an effect object then count down a timer. */
void func_8007EBA8(void)
{
    func_8006B328(0x64, 0xC8, 2, -1, 5, 5, 0x15E, 8, -0xC8, 0x190, -0xFA,
                  0x1F4, 0x32, 0x81, 0x81, 8, 0);
    if (--D_801B277C == 0)
    {
        D_801B2778 += 1;
    }
}
