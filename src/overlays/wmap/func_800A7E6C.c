#include "common.h"

extern s32 D_801B2E4C;
extern s32 D_801B2E48;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A7E6C(void)
{
    if (--D_801B2E4C == 0)
    {
        D_801B2E48 += 1;
    }
}
