#include "common.h"

extern s32 D_801B2BE0;
extern s32 D_8013B20C;
extern void func_80097DE0(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80097DA4(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2BE0 += 1;
        func_80097DE0();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80097DE0(void)
{
    D_801B2BE0 += 1;
}
