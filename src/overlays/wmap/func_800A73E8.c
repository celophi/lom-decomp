#include "common.h"

extern u32 D_801B2E70;
extern s32 D_801B2E74;
extern void (*D_800D6D34[])(void);
extern s32 D_801398D0;
extern void func_800A7440(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A7370(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2E70 = 1;
        D_801B2E74 = 1;
        return 1;
    }

    if (D_801B2E70 < 0xA)
    {
        D_800D6D34[D_801B2E70]();
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
void func_800A73E8(void)
{
    D_801B2E70 = 1;
    D_801B2E74 = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A7400(void)
{
    if (D_801398D0 != 2)
    {
        D_801B2E70 += 1;
        func_800A7440();
    }
}
