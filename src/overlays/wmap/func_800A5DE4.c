#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2E38;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B2E3C;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800A5D68(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x1F, 0x7, 0);
    if (--D_801B2E3C == 0)
    {
        D_801B2E38 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A5DE4(void)
{
    D_801B2E38 += 1;
}
