#include "common.h"

extern s32 D_801B2DDC;
extern s32 D_801B2DD8;
extern void func_8006CAC0(void (*step)(void));
extern void func_800A4C10(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A4088(void)
{
    if (--D_801B2DDC == 0)
    {
        D_801B2DD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800A40BC(void)
{
    func_8006CAC0(func_800A4C10);
    D_801B2DDC = 0xC;
    D_801B2DD8 += 1;
}
