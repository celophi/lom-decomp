#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80077C34(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80077868(void)
{
    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007789C(void)
{
    func_8006CAC0(func_80077C34);
    D_801B25F4 = 0x8;
    D_801B25F0 += 1;
}
