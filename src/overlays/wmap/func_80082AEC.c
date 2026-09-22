#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2818;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B281C;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082A70(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x18, 0x22, 0);
    if (--D_801B281C == 0)
    {
        D_801B2818 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80082AEC(void)
{
    D_801B2818 += 1;
}
