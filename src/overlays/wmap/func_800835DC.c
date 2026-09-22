#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_8011CF74;
extern s32 D_801B0FD0;
extern s32 D_801B2840;
extern s32 D_801B2844;

/** @brief Advance the effect on odd frames, draw it, and update the countdown. */
void func_800835DC(void)
{
    s32 remaining_ticks;

    if (D_8011CF74 & 1)
    {
        D_801B0FD0 += 1;
    }
    func_8006AFAC(0x13, 0x45, 0x13, 0x20, 0xFD0, 4, 0x20, 1);
    remaining_ticks = D_801B2844 - 1;
    D_801B2844 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2840 += 1;
    }
}
