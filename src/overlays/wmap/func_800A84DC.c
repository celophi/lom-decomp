#include "common.h"

extern s32 D_801B2E5C;
extern s32 D_801B2E58;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A84DC(void)
{
    if (--D_801B2E5C == 0)
    {
        D_801B2E58 += 1;
    }
}
