#include "common.h"

extern s32 D_801B28BC;
extern s32 D_801B28B8;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085D6C(void)
{
    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}
