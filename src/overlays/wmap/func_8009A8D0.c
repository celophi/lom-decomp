#include "common.h"

extern s32 D_801398AC;
extern s32 D_801B2C4C;
extern void func_8009A90C(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009A8D0(void)
{
    if (D_801398AC == 0)
    {
        D_801B2C4C += 1;
        func_8009A90C();
    }
}
