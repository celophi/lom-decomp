#include "common.h"

extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_8006CAC0(void (*step)(void));
extern void func_800AD360(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABB00(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800ABB34(void)
{
    func_8006CAC0(func_800AD360);
    D_801B2E84 = 0xC;
    D_801B2E80 += 1;
}
