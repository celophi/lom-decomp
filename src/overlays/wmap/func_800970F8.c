#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B2BC8;
extern s32 D_801B2BCC;

/** @brief World-map step handler: draw the sprite element, then countdown-advance the step. */
void func_800970F8(void)
{
    func_8006B6EC(0x8C, 0x90, 0xF, -6, 4);
    if (--D_801B2BCC == 0)
    {
        D_801B2BC8 += 1;
    }
}
