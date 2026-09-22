#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80095420(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009413C(void)
{
    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80094170(void)
{
    func_8006CAC0(func_80095420);
    D_801B2B34 = 0x14;
    D_801B2B30 += 1;
}
