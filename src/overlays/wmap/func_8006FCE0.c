#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_8006F564(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006FCE0(void)
{
    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8006FD14(void)
{
    func_8006CAC0(func_8006F564);
    D_801B2404 = 0x34;
    D_801B2400 += 1;
}
