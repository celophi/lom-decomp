#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B28D8;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B28DC;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80086654(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0xD, 0x3, 0);
    if (--D_801B28DC == 0)
    {
        D_801B28D8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800866D0(void)
{
    D_801B28D8 += 1;
}
