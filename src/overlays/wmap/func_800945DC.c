#include "common.h"

extern s32 D_801B2B34;
extern s32 D_801B2B30;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800945DC(void)
{
    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}
