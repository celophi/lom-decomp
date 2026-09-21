#include "common.h"

extern s32 D_801B2EFC;
extern s32 D_801B2EF8;
extern void func_8006CAC0(void (*step)(void));
extern void func_800B14D8(void);
extern void func_800B0074(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AF07C(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AF0B0(void)
{
    func_8006CAC0(func_800B14D8);
    func_8006CAC0(func_800B0074);
    D_801B2EFC = 0x20;
    D_801B2EF8 += 1;
}
