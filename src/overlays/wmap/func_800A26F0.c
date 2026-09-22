#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_8011CF4C;
extern s32 D_801B2D94;
extern s32 D_801B2D90;

/** @brief World-map step: init a sub-object then count down a timer. */
void func_800A26F0(void)
{
    s32 n = 0x8;

    func_8006CC4C(D_800D939C, D_801399C0);
    func_80066F9C(D_800D939C, D_8011CF4C, n, n, 0);
    if (--D_801B2D94 == 0)
    {
        D_801B2D90 += 1;
    }
}
