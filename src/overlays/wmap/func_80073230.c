#include "common.h"

extern s32 D_801B2514;
extern s32 D_801B2510;
extern void func_8006CAC0(void (*step)(void));
extern void func_80073C20(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80073230(void)
{
    if (--D_801B2514 == 0)
    {
        D_801B2510 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80073264(void)
{
    func_8006CAC0(func_80073C20);
    D_801B2514 = 0x2;
    D_801B2510 += 1;
}
