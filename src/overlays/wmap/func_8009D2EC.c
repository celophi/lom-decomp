#include "wmap_effect_primitives.h"
#include "common.h"

extern u32 D_801B2CC0;
extern s32 D_801B2CC4;
extern void (*D_800D6704[])(void);
extern u8 D_800DB158[];
extern u8 D_80139F28[];
extern s32 D_80139280;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009D274(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CC0 = 1;
        D_801B2CC4 = 1;
        return 1;
    }

    if (D_801B2CC0 < 0x6)
    {
        D_800D6704[D_801B2CC0]();
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
void func_8009D2EC(void)
{
    D_801B2CC0 = 1;
    D_801B2CC4 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009D304(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB158, D_80139F28, 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0xA0));
    if (--D_801B2CC4 == 0)
    {
        D_801B2CC0 += 1;
    }
}
