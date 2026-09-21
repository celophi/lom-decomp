#include "common.h"

extern s32 D_801B2BEC;
extern s32 D_801B2BE8;
extern void func_8006CAC0(void (*step)(void));
extern void func_8009858C(void);
extern void func_80098FC8(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098178(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800981AC(void)
{
    func_8006CAC0(func_8009858C);
    func_8006CAC0(func_80098FC8);
    D_801B2BEC = 0x8;
    D_801B2BE8 += 1;
}
