#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B24B8;
extern void func_800713D0(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80071394(void)
{
    if (D_8013B20C == 0)
    {
        D_801B24B8 += 1;
        func_800713D0();
    }
}
