#include "common.h"

extern u32 D_801B26C8;
extern s32 D_801B26CC;
extern void (*D_800D5378[])(void);

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007ABB0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B26C8 = 1;
        D_801B26CC = 1;
    }

    if (D_801B26C8 < 0x4)
    {
        D_800D5378[D_801B26C8]();
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
void func_8007AC20(void)
{
    D_801B26C8 = 1;
    D_801B26CC = 1;
}
