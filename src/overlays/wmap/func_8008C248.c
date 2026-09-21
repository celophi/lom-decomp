#include "common.h"

extern u32 D_801B29E8;
extern s32 D_801B29EC;
extern void (*D_800D5D48[])(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008C1D0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B29E8 = 1;
        D_801B29EC = 1;
        return 1;
    }

    if (D_801B29E8 < 0x6)
    {
        D_800D5D48[D_801B29E8]();
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
void func_8008C248(void)
{
    D_801B29E8 = 1;
    D_801B29EC = 1;
}
