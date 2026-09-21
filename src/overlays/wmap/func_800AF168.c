#include "common.h"

extern s32 D_801B2EFC;
extern s32 D_801B2EF8;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AF168(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}
