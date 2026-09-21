#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2F90;
extern void func_800B2218(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800B21DC(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2F90 += 1;
        func_800B2218();
    }
}
