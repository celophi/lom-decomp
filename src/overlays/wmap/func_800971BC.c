#include "common.h"

extern void func_8006B6EC(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4);
extern s32 D_801B2BC8;
extern s32 D_801B2BCC;

/** @brief World-map step handler: draw the sprite element, then countdown-advance the step. */
void func_800971BC(void)
{
    func_8006B6EC(0x8C, 0x90, 0xF, -6, 4);
    if (--D_801B2BCC == 0)
    {
        D_801B2BC8 += 1;
    }
}
