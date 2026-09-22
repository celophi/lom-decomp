#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_80089F64(void);
extern void func_8008A738(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008952C(void)
{
    if (--D_801B2964 == 0)
    {
        D_801B2960 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80089560(void)
{
    func_8006CAC0(func_80089F64);
    func_8006CAC0(func_8008A738);
    D_801B2964 = 0x68;
    D_801B2960 += 1;
}
