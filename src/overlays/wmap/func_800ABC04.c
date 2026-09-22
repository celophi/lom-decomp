#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACC88(void);
extern void func_800ACA40(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABC04(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800ABC38(void)
{
    func_8006CAC0(func_800ACC88);
    func_8006CAC0(func_800ACA40);
    D_801B2E84 = 0x14;
    D_801B2E80 += 1;
}
