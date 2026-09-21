#include "common.h"

extern s32 D_801B2BEC;
extern s32 D_801B2BE8;
extern void func_8006CAC0(void (*step)(void));
extern void func_800987F8(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098264(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80098298(void)
{
    func_8006CAC0(func_800987F8);
    D_801B2BEC = 0x80;
    D_801B2BE8 += 1;
}
