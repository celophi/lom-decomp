#include "common.h"

extern s32 D_801B2914;
extern s32 D_801B2910;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80087834(void)
{
    if (--D_801B2914 == 0)
    {
        D_801B2910 += 1;
    }
}
