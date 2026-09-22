#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_80139280;
extern s32 D_801B24E8;
extern s32 D_801B24EC;

/** @brief Draw the sequence effect and advance when its countdown expires. */
void func_800720C4(void)
{
    s32 remaining_ticks;

    func_8006BC44(0x14, 0x3C, D_80139280, 0);
    remaining_ticks = D_801B24EC - 1;
    D_801B24EC = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B24E8 += 1;
    }
}
