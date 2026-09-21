#include "common.h"

extern s32 D_801B2404;
extern s32 D_801B2400;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006FDC0(void)
{
    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}
