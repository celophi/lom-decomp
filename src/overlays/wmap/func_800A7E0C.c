#include "common.h"

extern s32 D_801398D0;
extern s32 D_801B2E48;
extern void func_800A7E4C(void);

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A7E0C(void)
{
    if (D_801398D0 != 2)
    {
        D_801B2E48 += 1;
        func_800A7E4C();
    }
}
