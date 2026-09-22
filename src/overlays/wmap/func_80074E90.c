#include "common.h"

extern u32 D_801B2580;
extern s32 D_801B2584;
extern void (*D_800D5050[])(void);

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80074E20(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2580 = 1;
        D_801B2584 = 1;
    }

    if (D_801B2580 < 0x4)
    {
        D_800D5050[D_801B2580]();
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
void func_80074E90(void)
{
    D_801B2580 = 1;
    D_801B2584 = 1;
}
