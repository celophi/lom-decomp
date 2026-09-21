#include "common.h"

extern s32 D_801B2914;
extern s32 D_801B2910;
extern void func_8006CAC0(void (*step)(void));
extern void func_80088008(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80087A18(void)
{
    if (--D_801B2914 == 0)
    {
        D_801B2910 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80087A4C(void)
{
    func_8006CAC0(func_80088008);
    D_801B2914 = 0x8;
    D_801B2910 += 1;
}
