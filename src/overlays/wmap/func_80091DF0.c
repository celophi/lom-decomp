#include "common.h"

extern s32 D_801B2AD0;
extern s32 D_801B2AD4;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80091DBC(void)
{
    if (--D_801B2AD4 == 0)
    {
        D_801B2AD0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80091DF0(void)
{
    D_801B2AD0 += 1;
}
