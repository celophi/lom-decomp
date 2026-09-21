#include "common.h"

extern u32 D_801B26A8;
extern s32 D_801B26AC;
extern void (*D_800D5328[])(void);

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007A600(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B26A8 = 1;
        D_801B26AC = 1;
    }

    if (D_801B26A8 < 0x6)
    {
        D_800D5328[D_801B26A8]();
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
void func_8007A670(void)
{
    D_801B26A8 = 1;
    D_801B26AC = 1;
}
