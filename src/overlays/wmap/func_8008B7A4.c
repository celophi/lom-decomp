#include "common.h"

extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8006CAC0(void (*step)(void));
extern void func_8008BB28(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008B7A4(void)
{
    if (--D_801B29B4 == 0)
    {
        D_801B29B0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008B7D8(void)
{
    func_8006CAC0(func_8008BB28);
    D_801B29B4 = 0x7A;
    D_801B29B0 += 1;
}
