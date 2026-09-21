#include "common.h"

extern s32 D_801B2B88;
extern s32 D_8013B20C;
extern void func_800960C4(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80096088(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2B88 += 1;
        func_800960C4();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800960C4(void)
{
    D_801B2B88 += 1;
}
