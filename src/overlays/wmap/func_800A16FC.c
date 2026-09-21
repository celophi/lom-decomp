#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2D50;
extern void func_800A1738(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A16FC(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2D50 += 1;
        func_800A1738();
    }
}
