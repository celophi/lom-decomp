#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B268C;
extern s32 D_801B2688;
extern void func_8007A478(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80079C4C(void)
{
    if (--D_801B268C == 0)
    {
        D_801B2688 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80079C80(void)
{
    func_8006CAC0(func_8007A478);
    D_801B268C = 0x4;
    D_801B2688 += 1;
}
