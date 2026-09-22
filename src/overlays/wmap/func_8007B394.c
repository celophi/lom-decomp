#include "wmap_sequence_runtime.h"
#include "common.h"
#include "sdk/libgte.h"

extern s32 D_80139870[];
extern SVECTOR D_8013B238;
extern VECTOR D_8011CF60;
extern s32 D_80182DE4;
extern s32 D_8011CF28;
extern s32 D_801B270C;
extern s32 D_801B2708;

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8007B394(void)
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

    if (D_80182DE4 != 0)
    {
        func_8006CD98(D_8011CF28, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE4);
        D_80182DE4 -= 0x4;
        if (D_80182DE4 < 0)
        {
            D_80182DE4 = 0;
        }
    }

    PopMatrix();
    if (--D_801B270C == 0)
    {
        D_801B2708 += 1;
    }
}
