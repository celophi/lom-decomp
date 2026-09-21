#include "common.h"

extern s32 D_801B268C;
extern s32 D_801B2688;
extern void func_8006CAC0(void (*step)(void));
extern void func_8007A600(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80079CBC(void)
{
    if (--D_801B268C == 0)
    {
        D_801B2688 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80079CF0(void)
{
    func_8006CAC0(func_8007A600);
    D_801B268C = 0x4;
    D_801B2688 += 1;
}
