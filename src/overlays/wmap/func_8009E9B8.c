#include "common.h"

extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8006CAC0(void (*step)(void));
extern void func_800A00C4(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E9B8(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E9EC(void)
{
    func_8006CAC0(func_800A00C4);
    D_801B2CDC = 0x22;
    D_801B2CD8 += 1;
}
