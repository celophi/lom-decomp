#include "common.h"

extern s32 D_801B2F9C;
extern s32 D_801B2F98;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B27A8(void)
{
    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}
