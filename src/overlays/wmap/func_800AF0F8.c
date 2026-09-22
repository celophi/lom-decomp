#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2EFC;
extern s32 D_801B2EF8;
extern void func_800B073C(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AF0F8(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800AF12C(void)
{
    func_8006CAC0(func_800B073C);
    D_801B2EFC = 0x20;
    D_801B2EF8 += 1;
}
