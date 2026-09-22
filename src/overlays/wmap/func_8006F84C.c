#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2438;
extern s32 D_801B243C;
extern void (*D_800D4D54[])(void);
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8006F7DC(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2438 = 1;
        D_801B243C = 1;
    }

    if (D_801B2438 < 0x6)
    {
        D_800D4D54[D_801B2438]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006F84C(void)
{
    D_801B2438 = 1;
    D_801B243C = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8006F864(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 8, 0x2E, 0);
    if (--D_801B243C == 0)
    {
        D_801B2438 += 1;
    }
}
