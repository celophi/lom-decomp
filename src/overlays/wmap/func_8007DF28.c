#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B274C;
extern s32 D_801B2748;
extern void func_8007E88C(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007DF28(void)
{
    if (--D_801B274C == 0)
    {
        D_801B2748 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007DF5C(void)
{
    func_8006CAC0(func_8007E88C);
    D_801B274C = 0x1E;
    D_801B2748 += 1;
}
