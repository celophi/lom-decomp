#include "common.h"

extern u32 D_801B2530;
extern s32 D_801B2534;
extern void (*D_800D4F50[])(void);

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_800739C8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2530 = 1;
        D_801B2534 = 1;
    }

    if (D_801B2530 < 0x4)
    {
        D_800D4F50[D_801B2530]();
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
void func_80073A38(void)
{
    D_801B2530 = 1;
    D_801B2534 = 1;
}
