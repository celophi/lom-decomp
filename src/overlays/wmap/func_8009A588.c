#include "common.h"

extern s32 D_801B2C50;
extern s32 D_801B2C4C;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009A588(void)
{
    if (--D_801B2C50 == 0)
    {
        D_801B2C4C += 1;
    }
}
