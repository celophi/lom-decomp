#include "common.h"

extern s32 D_801B2DD0;
extern s32 D_8013B20C;
extern void func_800A3E58(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A3E1C(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2DD0 += 1;
        func_800A3E58();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A3E58(void)
{
    D_801B2DD0 += 1;
}
