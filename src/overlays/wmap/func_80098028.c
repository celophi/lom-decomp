#include "common.h"

extern s32 D_801B2BEC;
extern s32 D_801B2BE8;
extern void func_8006CAC0(void (*step)(void));
extern void func_80098998(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098028(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009805C(void)
{
    func_8006CAC0(func_80098998);
    D_801B2BEC = 0x4;
    D_801B2BE8 += 1;
}
