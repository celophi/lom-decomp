#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009EF74(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E75C(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E790(void)
{
    func_8006CAC0(func_8009EF74);
    D_801B2CDC = 0x2;
    D_801B2CD8 += 1;
}
