#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_8008A210(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800894BC(void)
{
    if (--D_801B2964 == 0)
    {
        D_801B2960 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800894F0(void)
{
    func_8006CAC0(func_8008A210);
    D_801B2964 = 0x38;
    D_801B2960 += 1;
}
