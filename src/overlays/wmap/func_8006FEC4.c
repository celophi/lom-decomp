#include "common.h"

extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_8006CAC0(void (*step)(void));
extern void func_8006F9C0(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006FEC4(void)
{
    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8006FEF8(void)
{
    func_8006CAC0(func_8006F9C0);
    D_801B2404 = 0x2;
    D_801B2400 += 1;
}
