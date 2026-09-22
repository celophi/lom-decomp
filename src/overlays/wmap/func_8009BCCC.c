#include "common.h"

extern s32 D_801B2C6C;
extern s32 D_801B2C68;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BCCC(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}
