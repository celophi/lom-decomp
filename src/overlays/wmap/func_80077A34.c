#include "common.h"

extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_8006CAC0(void (*step)(void));
extern void func_80078128(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80077A34(void)
{
    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80077A68(void)
{
    func_8006CAC0(func_80078128);
    D_801B25F4 = 0xC;
    D_801B25F0 += 1;
}
