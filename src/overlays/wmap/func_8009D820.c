#include "common.h"
#include "sdk/libgte.h"

extern void func_8006CD98(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6);
extern s32 D_80139870[];
extern SVECTOR D_8013B238;
extern VECTOR D_8011CF60;
extern s32 D_80182DF0;
extern s32 D_8011CF1C;
extern s32 D_801B2CFC;
extern s32 D_801B2CF8;

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8009D820(void)
{
    MATRIX m;
    s32 x;

    x = D_80139870[2] - 0xDAC;
    D_80139870[2] = x;
    if (x < 0x2710)
    {
        D_80139870[2] = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_8013B238, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DF0 != 0)
    {
        func_8006CD98(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DF0);
        D_80182DF0 -= 0x2;
        if (D_80182DF0 < 0)
        {
            D_80182DF0 = 0;
        }
    }

    PopMatrix();
    if (--D_801B2CFC == 0)
    {
        D_801B2CF8 += 1;
    }
}
