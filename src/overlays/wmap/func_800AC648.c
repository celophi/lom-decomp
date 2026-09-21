#include "common.h"

extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_8006CAC0(void (*step)(void));
extern void func_800AD4B8(void);
extern void func_800ACDAC(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC648(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC67C(void)
{
    func_8006CAC0(func_800AD4B8);
    func_8006CAC0(func_800ACDAC);
    D_801B2E84 = 0xE;
    D_801B2E80 += 1;
}
