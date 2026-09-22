#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80077FD8(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800779C4(void)
{
    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800779F8(void)
{
    func_8006CAC0(func_80077FD8);
    D_801B25F4 = 0xC;
    D_801B25F0 += 1;
}
