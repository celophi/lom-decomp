#include "common.h"

extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8006CAC0(void (*step)(void));
extern void func_8008BF78(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008B814(void)
{
    if (--D_801B29B4 == 0)
    {
        D_801B29B0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008B848(void)
{
    func_8006CAC0(func_8008BF78);
    D_801B29B4 = 0x10;
    D_801B29B0 += 1;
}
