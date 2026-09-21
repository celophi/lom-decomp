#include "common.h"

extern s32 D_801B2C60;
extern s32 D_8013B20C;
extern void func_8009BA74(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009BA38(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2C60 += 1;
        func_8009BA74();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009BA74(void)
{
    D_801B2C60 += 1;
}
