#include "common.h"

extern s32 D_801398D0;
extern s32 D_801B2E50;
extern void func_800A8394(void);

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A8354(void)
{
    if (D_801398D0 != 2)
    {
        D_801B2E50 += 1;
        func_800A8394();
    }
}
