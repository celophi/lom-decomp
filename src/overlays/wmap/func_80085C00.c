#include "common.h"

extern s32 D_801B28B0;
extern s32 D_8013B20C;
extern void func_80085C00(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80085BC4(void)
{
    if (D_8013B20C == 0)
    {
        D_801B28B0 += 1;
        func_80085C00();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80085C00(void)
{
    D_801B28B0 += 1;
}
