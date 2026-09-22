#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2BEC;
extern s32 D_801B2BE8;
extern void func_800992C4(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098108(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009813C(void)
{
    func_8006CAC0(func_800992C4);
    D_801B2BEC = 0x1E;
    D_801B2BE8 += 1;
}
