#include "common.h"

extern s32 D_801B2A74;
extern s32 D_801B2A70;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80090208(void)
{
    if (--D_801B2A74 == 0)
    {
        D_801B2A70 += 1;
    }
}
