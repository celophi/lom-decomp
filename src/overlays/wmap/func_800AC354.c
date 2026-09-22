#include "common.h"

extern s32 D_801B2E84;
extern s32 D_801B2E80;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC354(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}
