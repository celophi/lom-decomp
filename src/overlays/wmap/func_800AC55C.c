#include "common.h"

extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_8006CAC0(void (*step)(void));
extern void func_800ACB64(void);
extern void func_800AD118(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC55C(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC590(void)
{
    func_8006CAC0(func_800ACB64);
    func_8006CAC0(func_800AD118);
    D_801B2E84 = 0x4;
    D_801B2E80 += 1;
}
