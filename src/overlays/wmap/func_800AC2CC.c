#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD610(void);
extern void func_800AD118(void);
extern void func_800ACC88(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC2CC(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC300(void)
{
    func_8006CAC0(func_800AD610);
    func_8006CAC0(func_800AD118);
    func_8006CAC0(func_800ACC88);
    D_801B2E84 = 0x18;
    D_801B2E80 += 1;
}
