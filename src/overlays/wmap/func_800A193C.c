#include "common.h"

extern s32 D_801B2D5C;
extern s32 D_801B2D58;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A193C(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}
