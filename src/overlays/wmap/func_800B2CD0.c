#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2FB0;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B2FB4;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800B2C54(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x2A, 0x2, 0);
    if (--D_801B2FB4 == 0)
    {
        D_801B2FB0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B2CD0(void)
{
    D_801B2FB0 += 1;
}
