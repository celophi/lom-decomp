#include "common.h"

extern u32 D_801B2588;
extern s32 D_801B258C;
extern void (*D_800D5060[])(void);

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_800759B4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2588 = 1;
        D_801B258C = 1;
    }

    if (D_801B2588 < 0x4)
    {
        D_800D5060[D_801B2588]();
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
void func_80075A24(void)
{
    D_801B2588 = 1;
    D_801B258C = 1;
}
