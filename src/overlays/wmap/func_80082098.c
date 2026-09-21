#include "common.h"

extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_8006CAC0(void (*step)(void));
extern void func_80083980(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082098(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800820CC(void)
{
    func_8006CAC0(func_80083980);
    D_801B27F4 = 0xA;
    D_801B27F0 += 1;
}
