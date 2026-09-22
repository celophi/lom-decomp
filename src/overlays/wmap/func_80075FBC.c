#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800D939C[];
extern s32 D_801399C0;
extern s32 D_8011CF4C;
extern s32 D_80182DE8;
extern s32 D_801B2598;
extern s32 D_801B259C;
extern void func_80066F9C(void *a, s32 b, s32 c, s32 d, s32 e);

/** @brief World-map step: build a sprite, decrement a shared budget, expire the timer. */
void func_80075FBC(void)
{
    func_8006CC4C(D_800D939C, &D_801399C0);
    func_80066F9C(D_800D939C, D_8011CF4C, 4, 0xB, 0);
    *(s16 *)(D_800D939C + 0x24) = (u16)D_80182DE8;
    *(s16 *)(D_800D939C + 0x22) = (u16)D_80182DE8;
    D_80182DE8 -= 4;
    if (D_80182DE8 < 0)
    {
        D_80182DE8 = 0;
    }
    if (--D_801B259C == 0)
    {
        D_801B2598 += 1;
    }
}
