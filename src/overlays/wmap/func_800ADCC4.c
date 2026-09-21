#include "common.h"
#include "sdk/libgte.h"

extern VECTOR D_80139870;
extern SVECTOR D_8013B238;
extern VECTOR D_8011CF60;
extern s32 D_80182DF0;
extern s32 D_8011CF24;
extern s32 func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_801B2F24;
extern s32 D_801B2F20;

/** @brief World-map step: spin the model matrix, draw the highlight, then tick the sub-counter. */
void func_800ADCC4(void)
{
    MATRIX m;

    D_80139870.vz -= 0xDAC;
    if (D_80139870.vz < 0x2710)
    {
        D_80139870.vz = 0x2710;
    }
    PushMatrix();
    RotMatrix(&D_8013B238, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);
    if (D_80182DF0 != 0)
    {
        func_800675F0(D_8011CF24, 0, 4, 0x35, 0x7800, 1, D_80182DF0, 0, 0, -1);
        D_80182DF0 -= 4;
        if (D_80182DF0 < 0)
        {
            D_80182DF0 = 0;
        }
    }
    PopMatrix();
    if (--D_801B2F24 == 0)
    {
        D_801B2F20 += 1;
    }
}
