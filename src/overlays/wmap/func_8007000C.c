#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_800702E0(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007000C(void)
{
    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80070040(void)
{
    func_8006CAC0(func_800702E0);
    D_801B2404 = 0x46;
    D_801B2400 += 1;
}
