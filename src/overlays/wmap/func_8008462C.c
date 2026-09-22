#include "common.h"

extern s32 D_801B2874;
extern s32 D_801B2870;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008462C(void)
{
    if (--D_801B2874 == 0)
    {
        D_801B2870 += 1;
    }
}
