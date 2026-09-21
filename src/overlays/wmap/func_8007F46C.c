#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2780;
extern void func_8007F4A8(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007F46C(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2780 += 1;
        func_8007F4A8();
    }
}
