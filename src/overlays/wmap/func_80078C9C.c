#include "common.h"

extern u32 D_801B2648;
extern s32 D_801B264C;
extern void (*D_800D5260[])(void);

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80078C2C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2648 = 1;
        D_801B264C = 1;
    }

    if (D_801B2648 < 0x8)
    {
        D_800D5260[D_801B2648]();
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
void func_80078C9C(void)
{
    D_801B2648 = 1;
    D_801B264C = 1;
}
