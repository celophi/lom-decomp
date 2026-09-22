#include "common.h"

extern s32 D_801B278C;
extern s32 D_801B2788;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007F728(void)
{
    if (--D_801B278C == 0)
    {
        D_801B2788 += 1;
    }
}
