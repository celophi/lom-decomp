#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80094D2C(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800942B8(void)
{
    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800942EC(void)
{
    func_8006CAC0(func_80094D2C);
    D_801B2B34 = 0x14;
    D_801B2B30 += 1;
}
