#include "common.h"

extern s32 D_801B274C;
extern s32 D_801B2748;
extern void func_8006CAC0(void (*step)(void));
extern void func_8007E480(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007DEB8(void)
{
    if (--D_801B274C == 0)
    {
        D_801B2748 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007DEEC(void)
{
    func_8006CAC0(func_8007E480);
    D_801B274C = 0x50;
    D_801B2748 += 1;
}
