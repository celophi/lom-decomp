#include "common.h"

extern u32 D_801B28B0;
extern s32 D_801B28B4;
extern void (*D_800D5960[])(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_8006C81C(void);
extern void func_80085B44(void);
extern s32 D_8013B20C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80085A70(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28B0 = 1;
        D_801B28B4 = 1;
        return 1;
    }

    if (D_801B28B0 < 0x6)
    {
        D_800D5960[D_801B28B0]();
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
void func_80085AE8(void)
{
    D_801B28B0 = 1;
    D_801B28B4 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80085B00(void)
{
    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B28B0 += 1;
    func_80085B44();
}
