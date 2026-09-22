#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2580;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;
extern s32 D_801B2584;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80074F20(void)
{
    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0xF, 0xB, 0);
    if (--D_801B2584 == 0)
    {
        D_801B2580 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80074F9C(void)
{
    D_801B2580 += 1;
}
