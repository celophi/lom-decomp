#include "common.h"

extern s32 D_801B2780;
extern s32 D_8013B20C;
extern void func_8007F528(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007F4EC(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2780 += 1;
        func_8007F528();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007F528(void)
{
    D_801B2780 += 1;
}
