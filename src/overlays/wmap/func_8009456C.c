#include "common.h"

extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_8006CAC0(void (*step)(void));
extern void func_80095724(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009456C(void)
{
    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800945A0(void)
{
    func_8006CAC0(func_80095724);
    D_801B2B34 = 0x34;
    D_801B2B30 += 1;
}
