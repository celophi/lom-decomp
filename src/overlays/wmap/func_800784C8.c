#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B262C;
extern s32 D_801B2628;

/** @brief World-map step: init a sub-object then count down a timer. */
void func_800784C8(void)
{
    s32 n = 0xB;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, n, n, 0);
    if (--D_801B262C == 0)
    {
        D_801B2628 += 1;
    }
}
