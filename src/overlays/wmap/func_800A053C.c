#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B2D40;
extern s32 D_801B2D44;

/** @brief World-map step handler: spawn a scripted actor and expire the timer. */
void func_800A053C(void)
{
    func_8006B328(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC,
                  0x8C, 1, 0x81, 1, 8, 1);
    if (--D_801B2D44 == 0)
    {
        D_801B2D40 += 1;
    }
}
