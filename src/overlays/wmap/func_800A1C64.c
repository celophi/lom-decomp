#include "common.h"

extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_8006CAC0(void (*step)(void));
extern void func_800A2784(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A1C64(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800A1C98(void)
{
    func_8006CAC0(func_800A2784);
    D_801B2D5C = 0x84;
    D_801B2D58 += 1;
}
