#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B26E0;
extern void func_8007BA20(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007B9E4(void)
{
    if (D_8013B20C == 0)
    {
        D_801B26E0 += 1;
        func_8007BA20();
    }
}
