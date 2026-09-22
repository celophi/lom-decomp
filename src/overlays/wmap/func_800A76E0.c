#include "common.h"

extern u32 D_801B2E40;
extern s32 D_801B2E44;
extern void (*D_800D6C14[])(void);
extern s32 D_801398D0;
extern void func_800A6540(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A7668(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2E40 = 1;
        D_801B2E44 = 1;
        return 1;
    }

    if (D_801B2E40 < 0x10)
    {
        D_800D6C14[D_801B2E40]();
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
void func_800A76E0(void)
{
    D_801B2E40 = 1;
    D_801B2E44 = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A76F8(void)
{
    if (D_801398D0 != 2)
    {
        D_801B2E40 += 1;
        func_800A6540();
    }
}
