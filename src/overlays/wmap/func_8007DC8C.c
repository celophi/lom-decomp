#include "common.h"

extern u32 D_801B2740;
extern s32 D_801B2744;
extern void (*D_800D54F0[])(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_8006C81C(void);
extern void func_8007DCE8(void);
extern s32 D_8013B20C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007DC14(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2740 = 1;
        D_801B2744 = 1;
        return 1;
    }

    if (D_801B2740 < 0x6)
    {
        D_800D54F0[D_801B2740]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007DC8C(void)
{
    D_801B2740 = 1;
    D_801B2744 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8007DCA4(void)
{
    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2740 += 1;
    func_8007DCE8();
}
