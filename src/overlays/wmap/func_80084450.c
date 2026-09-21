#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2868;
extern void func_8008448C(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80084450(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2868 += 1;
        func_8008448C();
    }
}
