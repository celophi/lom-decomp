#include "common.h"

extern u32 D_801B2620;
extern s32 D_801B2624;
extern void (*D_800D5200[])(void);

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80078278(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2620 = 1;
        D_801B2624 = 1;
    }

    if (D_801B2620 < 0x4)
    {
        D_800D5200[D_801B2620]();
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
void func_800782E8(void)
{
    D_801B2620 = 1;
    D_801B2624 = 1;
}
