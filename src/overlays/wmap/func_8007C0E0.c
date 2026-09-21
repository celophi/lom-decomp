#include "common.h"

extern void func_8007B0D8(s32);
extern s32 D_80139234;
extern s32 D_801B26F0;
extern s32 D_801B26F4;

/** @brief Reduce the effect value to a minimum of one and count down the sequence step. */
void func_8007C0E0(void)
{
    s32 value;
    s32 remaining_ticks;

    value = D_80139234 - 1;
    D_80139234 = value;
    if (value <= 0)
    {
        D_80139234 = 1;
    }
    func_8007B0D8(0x10000);
    remaining_ticks = D_801B26F4 - 1;
    D_801B26F4 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B26F0 += 1;
    }
}
