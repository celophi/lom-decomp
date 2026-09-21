#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2568;
extern void func_800750D0(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80075094(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2568 += 1;
        func_800750D0();
    }
}
