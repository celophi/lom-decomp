#include "common.h"

extern s32 D_801B29B4;
extern s32 D_801B29B0;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008B7A4(void)
{
    if (--D_801B29B4 == 0)
    {
        D_801B29B0 += 1;
    }
}
