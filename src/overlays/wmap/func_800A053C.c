#include "common.h"

extern s32 D_801B2D40;
extern s32 D_801B2D44;
extern void func_8006B328(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5,
                          s32 a6, s32 a7, s32 a8, s32 a9, s32 a10, s32 a11,
                          s32 a12, s32 a13, s32 a14, s32 a15, s32 a16);

/** @brief World-map step handler: spawn a scripted actor and expire the timer. */
void func_800A053C(void)
{
    func_8006B328(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC,
                  0x8C, 1, 0x81, 1, 8, 1);
    if (--D_801B2D44 == 0)
    {
        D_801B2D40 += 1;
    }
}
