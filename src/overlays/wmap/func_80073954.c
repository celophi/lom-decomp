#include "common.h"

extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_801B2520;
extern s32 D_801B2524;
extern void func_80072644(u8 *a0, u8 *a1, s32 a2);

/** @brief World-map step handler: run the sub-step, then advance after the timer. */
void func_80073954(void)
{
    func_80072644(D_800DB578, D_80139FE8, 0x18);
    if (--D_801B2524 == 0)
    {
        D_801B2520 += 1;
    }
}
