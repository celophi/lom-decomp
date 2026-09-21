#include "common.h"

extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8006CAC0(void (*step)(void));
extern void func_800A06FC(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E4AC(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E4E0(void)
{
    func_8006CAC0(func_800A06FC);
    D_801B2CDC = 0x18;
    D_801B2CD8 += 1;
}
