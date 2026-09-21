#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2958;
extern void func_80089244(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80089208(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2958 += 1;
        func_80089244();
    }
}
