#include "common.h"

extern void func_800773B8(s32, s32, s32);
extern s32 D_801B2648;
extern s32 D_801B264C;

/** @brief Update the sequence effect and advance when its countdown expires. */
void func_80078D64(void)
{
    s32 remaining_ticks;

    func_800773B8(0x7C, 0xAA, 0xD);
    remaining_ticks = D_801B264C - 1;
    D_801B264C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2648 += 1;
    }
}
