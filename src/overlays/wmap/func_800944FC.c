#include "common.h"

extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_8006CAC0(void (*step)(void));
extern void func_800948E0(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800944FC(void)
{
    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80094530(void)
{
    func_8006CAC0(func_800948E0);
    D_801B2B34 = 0x60;
    D_801B2B30 += 1;
}
