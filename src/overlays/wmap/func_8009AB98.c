#include "common.h"

extern u32 D_801B2C54;
extern s32 D_801B2C58;
extern void (*D_800D656C[])(void);
extern s32 D_801398D0;
extern void func_8009A258(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009AB20(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C54 = 1;
        D_801B2C58 = 1;
        return 1;
    }

    if (D_801B2C54 < 0xE)
    {
        D_800D656C[D_801B2C54]();
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
void func_8009AB98(void)
{
    D_801B2C54 = 1;
    D_801B2C58 = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_8009ABB0(void)
{
    if (D_801398D0 != 2)
    {
        D_801B2C54 += 1;
        func_8009A258();
    }
}
