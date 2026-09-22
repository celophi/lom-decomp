#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009F628(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E6EC(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E720(void)
{
    func_8006CAC0(func_8009F628);
    D_801B2CDC = 0x1C;
    D_801B2CD8 += 1;
}
