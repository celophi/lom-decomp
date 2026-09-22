#include "common.h"

extern s32 D_801B26E0;
extern s32 D_8013B20C;
extern void func_8007BAA0(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007BA64(void)
{
    if (D_8013B20C == 0)
    {
        D_801B26E0 += 1;
        func_8007BAA0();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007BAA0(void)
{
    D_801B26E0 += 1;
}
