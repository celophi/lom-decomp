#include "common.h"

extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_8006CAC0(void (*step)(void));
extern void func_80082500(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082178(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800821AC(void)
{
    func_8006CAC0(func_80082500);
    D_801B27F4 = 0x30;
    D_801B27F0 += 1;
}
