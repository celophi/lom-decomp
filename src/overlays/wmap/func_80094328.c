#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80094E80(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80094328(void)
{
    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009435C(void)
{
    func_8006CAC0(func_80094E80);
    D_801B2B34 = 0x28;
    D_801B2B30 += 1;
}
