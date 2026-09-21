#include "common.h"

extern s32 D_801B2964;
extern s32 D_801B2960;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800894BC(void)
{
    if (--D_801B2964 == 0)
    {
        D_801B2960 += 1;
    }
}
