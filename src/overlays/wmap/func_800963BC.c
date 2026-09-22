#include "common.h"

extern s32 D_801B2B94;
extern s32 D_801B2B90;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800963BC(void)
{
    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}
