#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2A74;
extern s32 D_801B2A70;
extern void func_80091334(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80090198(void)
{
    if (--D_801B2A74 == 0)
    {
        D_801B2A70 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800901CC(void)
{
    func_8006CAC0(func_80091334);
    D_801B2A74 = 0x32;
    D_801B2A70 += 1;
}
