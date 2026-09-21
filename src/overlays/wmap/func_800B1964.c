#include "common.h"
#include "sdk/libgte.h"

extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern s32 D_80139888[];
extern SVECTOR D_8013B240;
extern VECTOR D_8011CF60;
extern u8 D_800DCF18[];
extern s32 D_80182DF4;
extern s32 D_801B2FC0;
extern s32 D_801B2FC4;

/**
 * @brief World-map step handler: advance the model's spin toward a floor, draw it
 *        while active, then countdown-advance the step.
 */
void func_800B1964(void)
{
    MATRIX m;
    s32 x;

    x = D_80139888[2] - 0xDAC;
    D_80139888[2] = x;
    if (x < 0x2710)
    {
        D_80139888[2] = 0x2710;
    }
    PushMatrix();
    RotMatrix(&D_8013B240, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);
    if (D_80182DF4 != 0)
    {
        func_800675F0(D_800DCF18, 0, 0x4, 0x35, 0x7800, 1, D_80182DF4, 0, 0, -1);
        D_80182DF4 -= 0x20;
        if (D_80182DF4 < 0)
        {
            D_80182DF4 = 0;
        }
    }
    PopMatrix();
    if (--D_801B2FC4 == 0)
    {
        D_801B2FC0 += 1;
    }
}
