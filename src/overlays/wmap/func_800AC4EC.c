#include "common.h"

extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_8006CAC0(void (*step)(void));
extern void func_800ACC88(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC4EC(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800AC520(void)
{
    func_8006CAC0(func_800ACC88);
    D_801B2E84 = 0x14;
    D_801B2E80 += 1;
}
