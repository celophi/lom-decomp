#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B26D0;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B26D4;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007AE48(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0x14, 0xB, 0);
    if (--D_801B26D4 == 0)
    {
        D_801B26D0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007AEC4(void)
{
    D_801B26D0 += 1;
}
