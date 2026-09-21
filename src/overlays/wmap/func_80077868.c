#include "common.h"

extern s32 D_801B25F4;
extern s32 D_801B25F0;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80077868(void)
{
    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}
