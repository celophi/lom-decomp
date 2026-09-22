#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_8007CA6C(void);
extern s16 D_800D93C8[];
extern s32 D_801B271C;
extern s32 D_801B2718;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8007CA20(void)
{
    D_800D93C8[19] = 8;
    D_800D93C8[17] = 0;
    D_801B271C = 0x10;
    D_801B2718 += 1;
    func_8007CA6C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007CA6C(void)
{
    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x15, 0x5, 0);
    if (--D_801B271C == 0)
    {
        D_801B2718 += 1;
    }
}
