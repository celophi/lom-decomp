#include "common.h"
#include "sdk/libgte.h"

extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern u8 D_800DCF18[];
extern s32 D_80182DE8;
extern s32 D_801B2F10;
extern s32 D_801B2F14;

/**
 * @brief World-map step handler: advance the model's spin toward a floor, draw it
 *        while active, then countdown-advance the step.
 */
void func_800ADAA4(void)
{
    MATRIX m;
    s32 x;

    x = D_801B2650[2] - 0xDAC;
    D_801B2650[2] = x;
    if (x < 0x2710)
    {
        D_801B2650[2] = 0x2710;
    }
    PushMatrix();
    RotMatrix(&D_801B24A0, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);
    if (D_80182DE8 != 0)
    {
        func_800675F0(D_800DCF18, 0, 0x4, 0x35, 0x7800, 1, D_80182DE8, 0, 0, -1);
        D_80182DE8 -= 2;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();
    if (--D_801B2F14 == 0)
    {
        D_801B2F10 += 1;
    }
}
