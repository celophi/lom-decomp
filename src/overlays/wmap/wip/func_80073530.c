#include "common.h"

extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B2528;
extern s32 D_801B252C;

/**
 * @brief World-map step handler: draw the sprite, bump its animation field, then
 *        countdown-advance the step.
 * @note Best match ~97% (gcc280_g0); residual is the object base being reloaded via
 *       a distinct sub-object symbol (addiu -0xB0) that a single C symbol folds away.
 */
void func_80073530(void)
{
    u8 *obj;

    obj = D_800D9318;
    func_8006CC4C(obj, D_801399A8);
    func_80066F9C(obj, D_8011CF4C, 0xC, 0xA, 0);
    *(s16 *)(obj - 0xB0 + 0xD4) += 2;
    if (--D_801B252C == 0)
    {
        D_801B2528 += 1;
    }
}
