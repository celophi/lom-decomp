#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2AD0;
extern void func_80091D1C(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80091CE0(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2AD0 += 1;
        func_80091D1C();
    }
}
