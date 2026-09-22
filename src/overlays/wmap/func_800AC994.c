#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2E88;
extern s32 D_801B2E8C;
extern void (*D_800D6EFC[])(void);
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_80182D58;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AC91C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2E88 = 1;
        D_801B2E8C = 1;
        return 1;
    }

    if (D_801B2E88 < 0x4)
    {
        D_800D6EFC[D_801B2E88]();
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
void func_800AC994(void)
{
    D_801B2E88 = 1;
    D_801B2E8C = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800AC9AC(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_80182D58, 0x8, 0x2, 0);
    if (--D_801B2E8C == 0)
    {
        D_801B2E88 += 1;
    }
}
