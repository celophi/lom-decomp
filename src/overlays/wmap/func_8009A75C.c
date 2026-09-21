#include "common.h"

extern s32 D_801398AC;
extern s32 D_801B2C4C;
extern void func_8009A798(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009A75C(void)
{
    if (D_801398AC == 0)
    {
        D_801B2C4C += 1;
        func_8009A798();
    }
}
