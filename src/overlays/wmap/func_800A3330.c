#include "common.h"
#include "sdk/libgte.h"

extern s32 D_801B2DC8;
extern void func_8006CD98(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6);
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2DF4;
extern s32 D_801B2DF0;

/**
 * @brief Increment a world-map state counter.
 */
void func_800A3330(void)
{
    D_801B2DC8 += 1;
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_800A3348(void)
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
        func_8006CD98((s32)D_800DCF18, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
        D_80182DE8 -= 0x2;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B2DF4 == 0)
    {
        D_801B2DF0 += 1;
    }
}
