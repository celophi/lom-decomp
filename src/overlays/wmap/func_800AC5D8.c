#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD23C(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC5D8(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800AC60C(void)
{
    func_8006CAC0(func_800AD23C);
    D_801B2E84 = 0x8;
    D_801B2E80 += 1;
}
