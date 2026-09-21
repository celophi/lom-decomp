#include "common.h"

extern u32 D_801B2770;
extern s32 D_801B2774;
extern void (*D_800D5588[])(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007E88C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2770 = 1;
        D_801B2774 = 1;
        return 1;
    }

    if (D_801B2770 < 0x4)
    {
        D_800D5588[D_801B2770]();
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
void func_8007E904(void)
{
    D_801B2770 = 1;
    D_801B2774 = 1;
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007E91C(void)
{
    D_801B2770 += 1;
}
