#include "common.h"

extern s32 D_800D9164;
extern s32 D_801B2E70;
extern void func_800A7580(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A7544(void)
{
    if (D_800D9164 == 0)
    {
        D_801B2E70 += 1;
        func_800A7580();
    }
}
