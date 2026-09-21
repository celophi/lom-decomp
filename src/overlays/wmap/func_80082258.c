#include "common.h"

extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_8006CAC0(void (*step)(void));
extern void func_800836C8(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082258(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008228C(void)
{
    func_8006CAC0(func_800836C8);
    D_801B27F4 = 0x28;
    D_801B27F0 += 1;
}
