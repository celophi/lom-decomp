#include "common.h"

extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8006CAC0(void (*step)(void));
extern void func_8008C0A4(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008B884(void)
{
    if (--D_801B29B4 == 0)
    {
        D_801B29B0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008B8B8(void)
{
    func_8006CAC0(func_8008C0A4);
    D_801B29B4 = 0x68;
    D_801B29B0 += 1;
}
