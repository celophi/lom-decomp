#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2680;
extern void func_80079A9C(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80079A60(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2680 += 1;
        func_80079A9C();
    }
}
