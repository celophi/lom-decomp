#include "common.h"
#include "sdk/libgte.h"

extern s32 D_80139888[];
extern u8 D_8013B240[];
extern s32 D_8011CF28;
extern s32 D_80182DF4;
extern s32 D_801B2A38;
extern s32 D_801B2A3C;
extern void func_8006CFA8(void* a0, void* a1);
extern void func_8006CD98(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6);

/**
 * @brief World-map step handler: decay a timer with a floor, draw the model, ramp a
 *        secondary counter to its cap, scroll the field, then countdown-advance the step.
 */
void func_8008CC14(void)
{
    s32 v;

    v = D_80139888[2] - 0x5DC;
    D_80139888[2] = v;
    if (v < 0x7530)
    {
        D_80139888[2] = 0x7530;
    }
    PushMatrix();
    func_8006CFA8(D_80139888, D_8013B240);
    func_8006CD98(D_8011CF28, 0, 0xC, 0x35, 0x7840, 1, D_80182DF4);
    D_80182DF4 += 1;
    if (D_80182DF4 >= 0x42)
    {
        D_80182DF4 = 0x41;
    }
    ((u16*)D_8013B240)[2] += 2;
    PopMatrix();
    if (--D_801B2A3C == 0)
    {
        D_801B2A38 += 1;
    }
}
