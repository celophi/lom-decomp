#include "common.h"

extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80182DEC;
extern s32 D_801B2720;
extern s32 D_801B2724;
extern void func_8006D014(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6);

/**
 * @brief Draw a world-map element, then advance the step when its wait expires.
 */
void func_8007CC44(void)
{
    func_8006D014(D_800DB578, D_80139FE8, 8, 1, D_80182DEC, 8, 2);
    if (--D_801B2724 == 0)
    {
        D_801B2720 += 1;
    }
}
