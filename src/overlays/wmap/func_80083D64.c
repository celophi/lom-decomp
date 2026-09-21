#include "common.h"
#include "sdk/libgte.h"

extern void func_8006CFA8(void* a0, void* a1);
extern void func_8006CD98(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6);
extern u8 D_80182DC0[];
extern u8 D_801B2490[];
extern u8 D_801B2498[];
extern s32 D_8011CF1C;
extern s32 D_801B2468;
extern s32 D_801B288C;
extern s32 D_801B2888;

/**
 * @brief World-map step handler: draw two overlaid actor sprites within a matrix push,
 *        ramp the shared size up to a cap, then countdown-advance the step.
 */
void func_80083D64(void)
{
    s32 value;

    PushMatrix();
    func_8006CFA8(D_80182DC0, D_801B2490);
    func_8006CD98(D_8011CF1C, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    ((u16*)D_801B2490)[2] += 0xC;
    func_8006CFA8(D_80182DC0, D_801B2498);
    func_8006CD98(D_8011CF1C, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    ((u16*)D_801B2498)[2] += 0x4;
    PopMatrix();
    value = D_801B2468 + 2;
    D_801B2468 = value;
    if (value >= 0x82)
    {
        D_801B2468 = 0x81;
    }
    if (--D_801B288C == 0)
    {
        D_801B2888 += 1;
    }
}
