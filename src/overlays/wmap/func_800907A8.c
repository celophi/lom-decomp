#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800907F4(void);
extern s16 D_800D93C8[];
extern s32 D_801B2A84;
extern s32 D_801B2A80;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009072C(void)
{
    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x13, 0x2, 0);
    if (--D_801B2A84 == 0)
    {
        D_801B2A80 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800907A8(void)
{
    D_800D93C8[19] = 4;
    D_800D93C8[17] = 0;
    D_801B2A84 = 0x20;
    D_801B2A80 += 1;
    func_800907F4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800907F4(void)
{
    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x13, 0x2, 0);
    if (--D_801B2A84 == 0)
    {
        D_801B2A80 += 1;
    }
}
