#include "common.h"

extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_8006CAC0(void (*step)(void));
extern void func_80080DC4(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007F964(void)
{
    if (--D_801B278C == 0)
    {
        D_801B2788 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007F998(void)
{
    func_8006CAC0(func_80080DC4);
    D_801B278C = 0x10;
    D_801B2788 += 1;
}
