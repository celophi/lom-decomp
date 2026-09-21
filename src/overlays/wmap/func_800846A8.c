#include "common.h"

extern s32 D_801B2874;
extern s32 D_801B2870;
extern void func_8006CAC0(void (*step)(void));
extern void func_80084BDC(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800846A8(void)
{
    if (--D_801B2874 == 0)
    {
        D_801B2870 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800846DC(void)
{
    func_8006CAC0(func_80084BDC);
    D_801B2874 = 0x3C;
    D_801B2870 += 1;
}
