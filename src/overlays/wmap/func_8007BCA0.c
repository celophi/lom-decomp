#include "common.h"

extern s32 D_801B26EC;
extern s32 D_801B26E8;

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007BCA0(void)
{
    if (--D_801B26EC == 0)
    {
        D_801B26E8 += 1;
    }
}
