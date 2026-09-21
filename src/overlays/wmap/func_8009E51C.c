#include "common.h"

extern s32 D_801B2CDC;
extern s32 D_801B2CD8;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E51C(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}
