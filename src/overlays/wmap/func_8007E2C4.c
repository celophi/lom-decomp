#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2750;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B2754;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007E248(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0xF, 0x1E, 0);
    if (--D_801B2754 == 0)
    {
        D_801B2750 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007E2C4(void)
{
    D_801B2750 += 1;
}
