#include "wmap_effect_primitives.h"
#include "common.h"

extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 D_80139280;
extern s32 D_801B2938;
extern s32 D_801B293C;

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800881F0(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x32, 0, 0x7F, 0x2, 0, D_80139280);
    if (--D_801B293C == 0)
    {
        D_801B2938 += 1;
    }
}
