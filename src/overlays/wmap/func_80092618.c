#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_80092664(void);
extern s16 D_800D93C8[];
extern s32 D_801B2AEC;
extern s32 D_801B2AE8;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80092618(void)
{
    D_800D93C8[19] = 4;
    D_800D93C8[17] = 0;
    D_801B2AEC = 0x20;
    D_801B2AE8 += 1;
    func_80092664();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80092664(void)
{
    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x13, 0x14, 0);
    if (--D_801B2AEC == 0)
    {
        D_801B2AE8 += 1;
    }
}
