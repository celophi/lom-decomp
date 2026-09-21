#include "common.h"

extern s32 D_801B109C;
extern s32 D_801B1098;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006C930(void)
{
    if (--D_801B109C == 0)
    {
        D_801B1098 += 1;
    }
}
