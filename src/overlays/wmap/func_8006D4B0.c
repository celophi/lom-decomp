#include "common.h"

extern s32 D_801398D0;
extern s32 D_801B10A8;
extern void func_8006D4F0(void);

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_8006D4B0(void)
{
    if (D_801398D0 != 2)
    {
        D_801B10A8 += 1;
        func_8006D4F0();
    }
}
