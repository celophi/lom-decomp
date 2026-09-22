#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2A68;
extern s32 D_801B2A6C;
extern void (*D_800D5EE0[])(void);
extern void func_8008FDF0(void);
extern s32 D_8013B20C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008FD1C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2A68 = 1;
        D_801B2A6C = 1;
        return 1;
    }

    if (D_801B2A68 < 0x6)
    {
        D_800D5EE0[D_801B2A68]();
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
void func_8008FD94(void)
{
    D_801B2A68 = 1;
    D_801B2A6C = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8008FDAC(void)
{
    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2A68 += 1;
    func_8008FDF0();
}
