#include "common.h"

extern u32 D_801B2B88;
extern s32 D_801B2B8C;
extern void (*D_800D62A0[])(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_8006C81C(void);
extern void func_80096008(void);
extern s32 D_8013B20C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80095F34(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2B88 = 1;
        D_801B2B8C = 1;
        return 1;
    }

    if (D_801B2B88 < 0x6)
    {
        D_800D62A0[D_801B2B88]();
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
void func_80095FAC(void)
{
    D_801B2B88 = 1;
    D_801B2B8C = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80095FC4(void)
{
    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2B88 += 1;
    func_80096008();
}
