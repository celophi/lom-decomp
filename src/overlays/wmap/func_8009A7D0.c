#include "common.h"

extern void func_80099754(s32);
extern s16 D_800D926A;
extern s16 D_801AFBD2;
extern s32 D_801B2C4C;
extern s32 D_801B2C50;

/** @brief Update the effect and advance its sequence on completion or timeout. */
void func_8009A7D0(void)
{
    s32 remaining_ticks;

    func_80099754(1);
    if (D_801AFBD2 == 0x200)
    {
        D_800D926A = -1;
        D_801B2C50 = 0;
        D_801B2C4C += 1;
    }
    remaining_ticks = D_801B2C50 - 1;
    D_801B2C50 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2C4C += 1;
    }
}
