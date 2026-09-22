#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2A08;
extern s32 D_801B2A0C;
extern void (*D_800D5D98[])(void);
extern void func_8008D588(void);
extern s32 D_8013B20C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008D4B4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2A08 = 1;
        D_801B2A0C = 1;
        return 1;
    }

    if (D_801B2A08 < 0x6)
    {
        D_800D5D98[D_801B2A08]();
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
void func_8008D52C(void)
{
    D_801B2A08 = 1;
    D_801B2A0C = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8008D544(void)
{
    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2A08 += 1;
    func_8008D588();
}
