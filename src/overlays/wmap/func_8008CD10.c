#include "common.h"

extern u8 D_80139888[];
extern u8 D_8013B240[];
extern s32 D_80182DF4;
extern s32 D_8011CF28;
extern s32 D_801B2A38;
extern s32 D_801B2A3C;
extern void func_8006CFA8(void *arg0, void *arg1);
extern void func_8006CD98(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);

/**
 * @brief World-map step handler: clamp a fade level, draw a sprite, and expire the step.
 */
void func_8008CD10(void)
{
    u8 *p;
    u8 *q;

    p = D_80139888;
    if (*(s32 *)(p + 8) < 0x7530)
    {
        *(s32 *)(p + 8) = 0x7530;
    }
    PushMatrix();
    q = D_8013B240;
    func_8006CFA8(p, q);
    if (D_80182DF4 != 0)
    {
        func_8006CD98(D_8011CF28, 0, 0xC, 0x35, 0x7840, 1, D_80182DF4);
        D_80182DF4 -= 1;
        if (D_80182DF4 < 0)
        {
            D_80182DF4 = 0;
        }
        *(s16 *)(q + 4) += 2;
    }
    PopMatrix();
    if (--D_801B2A3C == 0)
    {
        D_801B2A38 += 1;
    }
}
