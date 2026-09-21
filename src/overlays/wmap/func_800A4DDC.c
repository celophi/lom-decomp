#include "common.h"

extern u32 D_801B2E10;
extern s32 D_801B2E14;
extern void (*D_800D6B6C[])(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A4D64(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2E10 = 1;
        D_801B2E14 = 1;
        return 1;
    }

    if (D_801B2E10 < 0x8)
    {
        D_800D6B6C[D_801B2E10]();
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
void func_800A4DDC(void)
{
    D_801B2E10 = 1;
    D_801B2E14 = 1;
}
