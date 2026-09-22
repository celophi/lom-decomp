#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_8008A540(void);
extern void func_80089B18(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80089618(void)
{
    if (--D_801B2964 == 0)
    {
        D_801B2960 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8008964C(void)
{
    func_8006CAC0(func_8008A540);
    func_8006CAC0(func_80089B18);
    D_801B2964 = 0x8B;
    D_801B2960 += 1;
}
