#include "common.h"

extern s32 D_801B2740;
extern s32 D_8013B20C;
extern void func_8007DDA4(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007DD68(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2740 += 1;
        func_8007DDA4();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007DDA4(void)
{
    D_801B2740 += 1;
}
