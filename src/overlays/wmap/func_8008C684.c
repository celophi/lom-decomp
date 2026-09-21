#include "common.h"

extern void func_8006B998(s32, s32, void *, s32, s32);
extern u8 D_800E4F18[];
extern s32 D_801B2A00;
extern s32 D_801B2A04;

/** @brief Draw the sequence effect and advance when its countdown expires. */
void func_8008C684(void)
{
    s32 remaining_ticks;

    func_8006B998(0x64, 0x7C, D_800E4F18, 8, 0xA);
    remaining_ticks = D_801B2A04 - 1;
    D_801B2A04 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2A00 += 1;
    }
}
