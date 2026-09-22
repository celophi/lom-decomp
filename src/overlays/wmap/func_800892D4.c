#include "common.h"

extern s32 D_801B2958;
extern s32 D_8013B20C;
extern void func_800892D4(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80089298(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2958 += 1;
        func_800892D4();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800892D4(void)
{
    D_801B2958 += 1;
}
