#include "common.h"

extern s32 D_801B2E54;
extern s32 D_801B2E50;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A80F4(void)
{
    if (--D_801B2E54 == 0)
    {
        D_801B2E50 += 1;
    }
}
