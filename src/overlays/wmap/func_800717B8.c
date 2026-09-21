#include "common.h"

extern s32 D_801B24C4;
extern s32 D_801B24C0;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800717B8(void)
{
    if (--D_801B24C4 == 0)
    {
        D_801B24C0 += 1;
    }
}
