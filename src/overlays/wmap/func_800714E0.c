#include "common.h"

extern u32 D_801B24C0;
extern s32 D_801B24C4;
extern void (*D_800D4DE8[])(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80071468(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B24C0 = 1;
        D_801B24C4 = 1;
        return 1;
    }

    if (D_801B24C0 < 0x14)
    {
        D_800D4DE8[D_801B24C0]();
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
void func_800714E0(void)
{
    D_801B24C0 = 1;
    D_801B24C4 = 1;
}
