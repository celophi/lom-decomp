#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2EFC;
extern s32 D_801B2EF8;
extern void func_800B032C(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AF41C(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800AF450(void)
{
    func_8006CAC0(func_800B032C);
    D_801B2EFC = 0x2;
    D_801B2EF8 += 1;
}
