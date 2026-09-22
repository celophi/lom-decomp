#include "common.h"

extern u32 D_801B2910;
extern s32 D_801B2914;
extern void (*D_800D5A78[])(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80087744(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2910 = 1;
        D_801B2914 = 1;
        return 1;
    }

    if (D_801B2910 < 0x10)
    {
        D_800D5A78[D_801B2910]();
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
void func_800877BC(void)
{
    D_801B2910 = 1;
    D_801B2914 = 1;
}
