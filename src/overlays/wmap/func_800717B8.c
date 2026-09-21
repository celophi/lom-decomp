#include "common.h"

extern s32 D_801B24C4;
extern s32 D_801B24C0;
extern void func_8006CAC0(void (*step)(void));
extern void func_80072538(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800717B8(void)
{
    if (--D_801B24C4 == 0)
    {
        D_801B24C0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800717EC(void)
{
    func_8006CAC0(func_80072538);
    D_801B24C4 = 0x1E;
    D_801B24C0 += 1;
}
