#include "common.h"

extern s32 D_801B2574;
extern s32 D_801B2570;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80075338(void)
{
    if (--D_801B2574 == 0)
    {
        D_801B2570 += 1;
    }
}
