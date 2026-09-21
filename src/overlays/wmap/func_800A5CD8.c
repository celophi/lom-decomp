#include "common.h"

extern u32 D_801B2E38;
extern s32 D_801B2E3C;
extern void (*D_800D6C04[])(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A5C60(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2E38 = 1;
        D_801B2E3C = 1;
        return 1;
    }

    if (D_801B2E38 < 0x4)
    {
        D_800D6C04[D_801B2E38]();
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
void func_800A5CD8(void)
{
    D_801B2E38 = 1;
    D_801B2E3C = 1;
}
