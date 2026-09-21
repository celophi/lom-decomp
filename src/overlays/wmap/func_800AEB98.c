#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2EF0;
extern void func_800AEBD4(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800AEB98(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2EF0 += 1;
        func_800AEBD4();
    }
}
