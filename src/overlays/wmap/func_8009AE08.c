/* Partial WMAP decompilation: 89.027020% (gcc280_g0). */
#include "common.h"

extern u8 D_800DBE3C[];
extern s32 D_8013A180;
extern s32 D_8011CF54;
extern s16 D_801AFBE0;
extern s32 D_801B2C54;
extern s32 D_801B2C58;
extern void func_80099754(s32 arg);
extern void func_8006CC4C(void *dst, void *src);
extern void func_80066F9C(void *a, s32 b, s32 c, s32 d, s32 e);

/**
 * @brief World-map step: build a sprite, decrement a shared budget, expire the timer.
 * @note Best match ~89.03% (gcc280_g0); residual is prologue scheduling of the
 *       s0 save versus the first call (permuter territory).
 */
void func_8009AE08(void)
{
    func_80099754(0);
    func_8006CC4C(D_800DBE3C, &D_8013A180);
    func_80066F9C(D_800DBE3C, D_8011CF54, 0x28, D_801AFBE0, 2);
    *(s16 *)&D_8011CF54 = *(u16 *)&D_8011CF54 - 4;
    if (--D_801B2C58 == 0)
    {
        D_801B2C54 += 1;
    }
}
