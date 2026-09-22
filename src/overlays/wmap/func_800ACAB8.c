#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2E90;
extern s32 D_801B2E94;
extern void (*D_800D6F0C[])(void);
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_80182D60;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACA40(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2E90 = 1;
        D_801B2E94 = 1;
        return 1;
    }

    if (D_801B2E90 < 0x4)
    {
        D_800D6F0C[D_801B2E90]();
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
void func_800ACAB8(void)
{
    D_801B2E90 = 1;
    D_801B2E94 = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACAD0(void)
{
    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_80182D60, 0x20, 0x2, 0);
    if (--D_801B2E94 == 0)
    {
        D_801B2E90 += 1;
    }
}
