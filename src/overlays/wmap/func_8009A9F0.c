#include "common.h"

extern s32 D_800D9164;
extern s32 D_801B2C4C;
extern void func_8009AA2C(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009A9F0(void)
{
    if (D_800D9164 == 0)
    {
        D_801B2C4C += 1;
        func_8009AA2C();
    }
}
