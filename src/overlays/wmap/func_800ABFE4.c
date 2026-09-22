#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AC91C(void);
extern void func_800ACED0(void);
extern void func_800AD610(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABFE4(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC018(void)
{
    func_8006CAC0(func_800AC91C);
    func_8006CAC0(func_800ACED0);
    func_8006CAC0(func_800AD610);
    D_801B2E84 = 0x4;
    D_801B2E80 += 1;
}
