#include "wmap_effect_primitives.h"
#include "common.h"

extern void *D_80139280;
extern s32 D_801B2C1C;
extern s32 D_801B2C18;

/** @brief World-map step: kick off a descriptor animation, then tick the sub-counter. */
void func_80098CD8(void)
{
    func_8006BC44(0x14, 0x3C, D_80139280, 1);
    if (--D_801B2C1C == 0)
    {
        D_801B2C18 += 1;
    }
}
