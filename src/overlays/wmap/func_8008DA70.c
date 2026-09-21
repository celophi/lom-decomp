#include "common.h"

extern s32 D_801B2A14;
extern s32 D_801B2A10;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008DA70(void)
{
    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}
