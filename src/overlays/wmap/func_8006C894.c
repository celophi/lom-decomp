#include "common.h"

extern u32 D_801B1098;
extern s32 D_801B109C;
extern void (*D_800D0A44[])(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or a reset occurred, 0 if the index was out of range.
 */
s32 func_8006C81C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B1098 = 1;
        D_801B109C = 1;
        return 1;
    }

    if (D_801B1098 < 0xA)
    {
        D_800D0A44[D_801B1098]();
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
void func_8006C894(void)
{
    D_801B1098 = 1;
    D_801B109C = 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006C8AC(void)
{
    if (--D_801B109C == 0)
    {
        D_801B1098 += 1;
    }
}
