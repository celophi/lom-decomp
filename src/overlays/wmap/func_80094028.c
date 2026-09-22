#include "common.h"

extern s32 D_801B2B28;
extern s32 D_8013B20C;
extern void func_80094028(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80093FEC(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2B28 += 1;
        func_80094028();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80094028(void)
{
    D_801B2B28 += 1;
}
