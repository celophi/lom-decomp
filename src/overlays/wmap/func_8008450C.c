#include "common.h"

extern s32 D_801B2868;
extern s32 D_8013B20C;
extern void func_8008450C(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800844D0(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2868 += 1;
        func_8008450C();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008450C(void)
{
    D_801B2868 += 1;
}
