#include "common.h"

extern void func_8007B0D8(s32);
extern s32 D_801B26F0;
extern s32 D_801B26F4;

/** @brief Update the sequence effect and advance when its countdown reaches zero. */
void func_8007C05C(void)
{
    s32 remaining_ticks;

    func_8007B0D8(0x10000);
    remaining_ticks = D_801B26F4 - 1;
    D_801B26F4 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B26F0 += 1;
    }
}
