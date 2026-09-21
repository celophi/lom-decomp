#include "common.h"

extern s32 D_801B2518;
extern s32 D_8013B20C;
extern void func_80073830(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800737F4(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2518 += 1;
        func_80073830();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80073830(void)
{
    D_801B2518 += 1;
}
