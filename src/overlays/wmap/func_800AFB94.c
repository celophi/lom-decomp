#include "common.h"

extern u32 D_801B2F18;
extern s32 D_801B2F1C;
extern void (*D_800D70AC[])(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AFB1C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F18 = 1;
        D_801B2F1C = 1;
        return 1;
    }

    if (D_801B2F18 < 0x4)
    {
        D_800D70AC[D_801B2F18]();
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
void func_800AFB94(void)
{
    D_801B2F18 = 1;
    D_801B2F1C = 1;
}
