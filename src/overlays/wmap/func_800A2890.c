#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;
extern s32 D_801B2D9C;
extern s32 D_801B2D98;
extern void func_80066F9C(void *a, s32 b, s32 c, s32 d, s32 e);

/** @brief World-map step: init a sub-object then count down a timer. */
void func_800A2890(void)
{
    s32 n = 0x8;

    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, n, n, 0);
    if (--D_801B2D9C == 0)
    {
        D_801B2D98 += 1;
    }
}
