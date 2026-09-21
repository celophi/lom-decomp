#include "common.h"

extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_8006CAC0(void (*step)(void));
extern void func_80089E10(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800895A8(void)
{
    if (--D_801B2964 == 0)
    {
        D_801B2960 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800895DC(void)
{
    func_8006CAC0(func_80089E10);
    D_801B2964 = 0x3C;
    D_801B2960 += 1;
}
