#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800B10FC(void);
extern s16 D_800D93F4[];
extern s32 D_801B2F7C;
extern s32 D_801B2F78;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800B10B0(void)
{
    D_800D93F4[19] = 2;
    D_800D93F4[17] = 0;
    D_801B2F7C = 0x40;
    D_801B2F78 += 1;
    func_800B10FC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800B10FC(void)
{
    func_8006CC4C(D_800D93F4, D_801399D0);
    func_80066F9C(D_800D93F4, D_8011CF4C, 0x8, 0x5, 0);
    if (--D_801B2F7C == 0)
    {
        D_801B2F78 += 1;
    }
}
