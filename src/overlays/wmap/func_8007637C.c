#include "common.h"

extern u32 D_801B25B0;
extern s32 D_801B25B4;
extern void (*D_800D50D0[])(void);

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007630C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B25B0 = 1;
        D_801B25B4 = 1;
    }

    if (D_801B25B0 < 0x6)
    {
        D_800D50D0[D_801B25B0]();
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
void func_8007637C(void)
{
    D_801B25B0 = 1;
    D_801B25B4 = 1;
}
