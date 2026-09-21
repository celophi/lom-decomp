#include "common.h"
#include "sdk/libgte.h"

extern void func_8006CFA8(void* a0, void* a1);
extern void func_8006CD98(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6);
extern u8 D_80182DC0[];
extern u8 D_801B2490[];
extern u8 D_801B2498[];
extern s32 D_8011CF24;
extern s32 D_801B2468;
extern s32 D_801B2B58;
extern s32 D_801B2B5C;

/**
 * @brief World-map step handler: draw two frames of the animated actor, scroll each
 *        sub-field, decay the shared frame index with a floor, then advance the step.
 */
void func_80093784(void)
{
    PushMatrix();
    func_8006CFA8(D_80182DC0, D_801B2490);
    func_8006CD98(D_8011CF24, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    ((u16*)D_801B2490)[2] += 0x20;
    func_8006CFA8(D_80182DC0, D_801B2498);
    func_8006CD98(D_8011CF24, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    ((u16*)D_801B2498)[2] += 0xC;
    PopMatrix();
    D_801B2468 -= 4;
    if (D_801B2468 < 0)
    {
        D_801B2468 = 0;
    }
    if (--D_801B2B5C == 0)
    {
        D_801B2B58 += 1;
    }
}
