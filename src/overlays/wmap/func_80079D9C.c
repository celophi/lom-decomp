#include "common.h"

extern s32 D_801B268C;
extern s32 D_801B2688;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80079D9C(void)
{
    if (--D_801B268C == 0)
    {
        D_801B2688 += 1;
    }
}
