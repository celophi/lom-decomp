#include "common.h"

extern u32 D_801B2C68;
extern s32 D_801B2C6C;
extern void (*D_800D65BC[])(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009BA8C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C68 = 1;
        D_801B2C6C = 1;
        return 1;
    }

    if (D_801B2C68 < 0x18)
    {
        D_800D65BC[D_801B2C68]();
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
void func_8009BB04(void)
{
    D_801B2C68 = 1;
    D_801B2C6C = 1;
}
