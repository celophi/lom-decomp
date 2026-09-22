#include "common.h"

extern s32 D_801B2680;
extern s32 D_8013B20C;
extern void func_80079B1C(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80079AE0(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2680 += 1;
        func_80079B1C();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80079B1C(void)
{
    D_801B2680 += 1;
}
