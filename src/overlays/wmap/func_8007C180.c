#include "common.h"

extern void func_8007B0D8(s32);
extern s32 D_8013923C;
extern s32 D_801B26F0;
extern s32 D_801B26F4;

/** @brief Reduce the effect value toward zero and advance when its countdown expires. */
void func_8007C180(void)
{
    s32 value;
    s32 remaining_ticks;

    value = D_8013923C - 8;
    D_8013923C = value;
    if (value < 0)
    {
        D_8013923C = 0;
    }
    func_8007B0D8(1);
    remaining_ticks = D_801B26F4 - 1;
    D_801B26F4 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B26F0 += 1;
    }
}
