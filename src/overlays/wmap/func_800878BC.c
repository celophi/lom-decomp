#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2914;
extern s32 D_801B2910;
extern void func_80088350(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800878BC(void)
{
    if (--D_801B2914 == 0)
    {
        D_801B2910 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800878F0(void)
{
    func_8006CAC0(func_80088350);
    D_801B2914 = 0x10;
    D_801B2910 += 1;
}
