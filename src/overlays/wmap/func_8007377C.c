#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2518;
extern void func_800737B8(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007377C(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2518 += 1;
        func_800737B8();
    }
}
