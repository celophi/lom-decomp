#include "common.h"

extern s32 D_801398D0;
extern s32 D_801B2E58;
extern void func_800A87B0(void);

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A8770(void)
{
    if (D_801398D0 != 2)
    {
        D_801B2E58 += 1;
        func_800A87B0();
    }
}
