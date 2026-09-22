#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_800A2F38(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A1850(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800A1884(void)
{
    func_8006CAC0(func_800A2F38);
    D_801B2D5C = 0x14;
    D_801B2D58 += 1;
}
