#include "common.h"

extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800981F4(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}
