#include "common.h"

extern s32 D_801B25E8;
extern s32 D_8013B20C;
extern void func_800776F0(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800776B4(void)
{
    if (D_8013B20C == 0)
    {
        D_801B25E8 += 1;
        func_800776F0();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800776F0(void)
{
    D_801B25E8 += 1;
}
