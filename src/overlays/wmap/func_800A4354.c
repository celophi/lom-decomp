#include "common.h"

extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A4354(void)
{
    if (--D_801B2DDC == 0)
    {
        D_801B2DD8 += 1;
    }
}
