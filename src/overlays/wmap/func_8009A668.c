#include "common.h"

extern s32 D_801398D0;
extern s32 D_801B2C4C;
extern void func_80099848(void);

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_8009A668(void)
{
    if (D_801398D0 != 2)
    {
        D_801B2C4C += 1;
        func_80099848();
    }
}
