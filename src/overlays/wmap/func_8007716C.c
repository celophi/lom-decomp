#include "common.h"
#include "sdk/libgte.h"

extern void func_8006CFA8(s32 *a0, s32 *a1);
extern void func_8006CD98(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6);
extern s32 D_801B2660[];
extern s32 D_801B2678;
extern s32 D_80182DEC;
extern s32 D_8011CF30;
extern s32 D_801B2624;
extern s32 D_801B2620;

/** @brief World-map step handler: decay a value, draw the model while active, then countdown-advance. */
void func_8007716C(void)
{
    s32 v1;

    v1 = D_801B2660[2] - 0x5DC;
    D_801B2660[2] = v1;
    if (v1 < 0x9C40)
    {
        D_801B2660[2] = 0x9C40;
    }

    PushMatrix();
    func_8006CFA8(D_801B2660, &D_801B2678);

    if (D_80182DEC != 0)
    {
        func_8006CD98(D_8011CF30, 0, 0xC, 0x35, 0x7800, 1, D_80182DEC);
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--D_801B2624 == 0)
    {
        D_801B2620 += 1;
    }
}
