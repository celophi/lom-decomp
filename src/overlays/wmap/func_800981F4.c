#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2BEC;
extern s32 D_801B2BE8;
extern void func_8009954C(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800981F4(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80098228(void)
{
    func_8006CAC0(func_8009954C);
    D_801B2BEC = 0xF;
    D_801B2BE8 += 1;
}
