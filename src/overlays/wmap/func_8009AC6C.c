#include "common.h"

extern s32 D_801398AC;
extern s32 D_801B2C54;
extern void func_8009ACA8(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009AC6C(void)
{
    if (D_801398AC == 0)
    {
        D_801B2C54 += 1;
        func_8009ACA8();
    }
}
