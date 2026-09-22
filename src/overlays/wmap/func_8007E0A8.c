#include "common.h"

extern s32 D_801B274C;
extern s32 D_801B2748;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007E0A8(void)
{
    if (--D_801B274C == 0)
    {
        D_801B2748 += 1;
    }
}
