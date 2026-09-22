#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2C20;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B2C24;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80098F34(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x18, 0x8, 0);
    if (--D_801B2C24 == 0)
    {
        D_801B2C20 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80098FB0(void)
{
    D_801B2C20 += 1;
}
