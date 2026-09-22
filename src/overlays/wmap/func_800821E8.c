#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_80082950(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800821E8(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008221C(void)
{
    func_8006CAC0(func_80082950);
    D_801B27F4 = 0x2;
    D_801B27F0 += 1;
}
