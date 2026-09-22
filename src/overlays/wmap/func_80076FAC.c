#include "wmap_sequence_runtime.h"
#include "common.h"
#include "sdk/libgte.h"

extern s32 D_80139888[];
extern s32 D_8013B240;
extern s32 D_80182DE4;
extern s32 D_8011CF28;
extern s32 D_801B2614;
extern s32 D_801B2610;

/** @brief World-map step handler: decay a value, draw the model while active, then countdown-advance. */
void func_80076FAC(void)
{
    s32 v1;

    v1 = D_80139888[2] - 0x5DC;
    D_80139888[2] = v1;
    if (v1 < 0x9C40)
    {
        D_80139888[2] = 0x9C40;
    }

    PushMatrix();
    func_8006CFA8(D_80139888, &D_8013B240);

    if (D_80182DE4 != 0)
    {
        func_8006CD98(D_8011CF28, 0, 0xC, 0x35, 0x7800, 1, D_80182DE4);
        if (D_80182DE4 < 0)
        {
            D_80182DE4 = 0;
        }
    }

    PopMatrix();
    if (--D_801B2614 == 0)
    {
        D_801B2610 += 1;
    }
}
