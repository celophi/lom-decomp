#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2B94;
extern s32 D_801B2B90;
extern void func_800966F0(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009644C(void)
{
    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80096480(void)
{
    func_8006CAC0(func_800966F0);
    D_801B2B94 = 0xAB;
    D_801B2B90 += 1;
}
