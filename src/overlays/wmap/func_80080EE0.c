#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_8011CF74;
extern s32 D_801B0FD0;
extern s32 D_801B27E0;
extern s32 D_801B27E4;

/** @brief Advance the effect on odd frames, draw it, and update the countdown. */
void func_80080EE0(void)
{
    s32 remaining_ticks;

    if (D_8011CF74 & 1)
    {
        D_801B0FD0 += 1;
    }
    func_8006AFAC(0xCC, 0xD2, 0x13, 0x20, 0x650, 8, 0x60, 0);
    remaining_ticks = D_801B27E4 - 1;
    D_801B27E4 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B27E0 += 1;
    }
}
