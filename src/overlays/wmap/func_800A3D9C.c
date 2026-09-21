#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2DD0;
extern void func_800A3DD8(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A3D9C(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2DD0 += 1;
        func_800A3DD8();
    }
}
