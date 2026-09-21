#include "common.h"

extern s32 D_801B27F4;
extern s32 D_801B27F0;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082010(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}
