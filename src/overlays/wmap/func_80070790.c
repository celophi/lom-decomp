#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2408;
extern void func_800707CC(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80070790(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2408 += 1;
        func_800707CC();
    }
}
