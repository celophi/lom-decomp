#include "common.h"
#include "sdk/libgte.h"

extern s32 D_801B2660[];
extern u8 D_801B2678[];
extern s32 D_8011CF30;
extern s32 D_801B25DC;
extern s32 D_801B2A48;
extern s32 D_801B2A4C;
extern void func_8006CFA8(void* a0, void* a1);
extern void func_8006CD98(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6);

/**
 * @brief World-map step handler: decay a timer with a floor, draw the model, ramp a
 *        secondary counter to its cap, scroll the field, then countdown-advance the step.
 */
void func_8008CFFC(void)
{
    s32 v;

    v = D_801B2660[2] - 0x5DC;
    D_801B2660[2] = v;
    if (v < 0x7530)
    {
        D_801B2660[2] = 0x7530;
    }
    PushMatrix();
    func_8006CFA8(D_801B2660, D_801B2678);
    func_8006CD98(D_8011CF30, 0, 0xC, 0x35, 0x7840, 1, D_801B25DC);
    D_801B25DC += 1;
    if (D_801B25DC >= 0x42)
    {
        D_801B25DC = 0x41;
    }
    ((u16*)D_801B2678)[2] += 4;
    PopMatrix();
    if (--D_801B2A4C == 0)
    {
        D_801B2A48 += 1;
    }
}
