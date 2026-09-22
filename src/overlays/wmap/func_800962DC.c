#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2B94;
extern s32 D_801B2B90;
extern void func_80096D38(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800962DC(void)
{
    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80096310(void)
{
    func_8006CAC0(func_80096D38);
    D_801B2B94 = 0x8;
    D_801B2B90 += 1;
}
