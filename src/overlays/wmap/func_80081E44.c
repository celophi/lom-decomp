#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_80082B04(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80081E44(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80081E78(void)
{
    func_8006CAC0(func_80082B04);
    D_801B27F4 = 0x4;
    D_801B27F0 += 1;
}
