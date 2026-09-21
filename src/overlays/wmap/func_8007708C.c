#include "common.h"
#include "sdk/libgte.h"

extern void func_8006CFA8(s32 *a0, s32 *a1);
extern void func_8006CD98(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6);
extern s32 D_80139898[];
extern s32 D_801B2670;
extern s32 D_80182DE8;
extern s32 D_8011CF2C;
extern s32 D_801B261C;
extern s32 D_801B2618;

/** @brief World-map step handler: decay a value, draw the model while active, then countdown-advance. */
void func_8007708C(void)
{
    s32 v1;

    v1 = D_80139898[2] - 0x5DC;
    D_80139898[2] = v1;
    if (v1 < 0x9C40)
    {
        D_80139898[2] = 0x9C40;
    }

    PushMatrix();
    func_8006CFA8(D_80139898, &D_801B2670);

    if (D_80182DE8 != 0)
    {
        func_8006CD98(D_8011CF2C, 0, 0xC, 0x35, 0x7800, 1, D_80182DE8);
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B261C == 0)
    {
        D_801B2618 += 1;
    }
}
