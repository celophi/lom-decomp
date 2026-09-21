#include "common.h"

extern u32 D_801AFBA0;
extern s32 D_801AFBA4;
extern void (*D_800D0458[])(void);

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8006544C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801AFBA0 = 1;
        D_801AFBA4 = 1;
    }

    if (D_801AFBA0 < 0x2)
    {
        D_800D0458[D_801AFBA0]();
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
void func_800654BC(void)
{
    D_801AFBA0 = 1;
    D_801AFBA4 = 1;
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800654D4(void)
{
    D_801AFBA0 += 1;
}
