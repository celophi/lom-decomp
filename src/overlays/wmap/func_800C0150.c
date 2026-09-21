#include "common.h"

extern s32 D_80139234;
extern s32 D_801B3220;
extern void func_800C018C(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800C0150(void)
{
    if (D_80139234 == 0)
    {
        D_801B3220 += 1;
        func_800C018C();
    }
}
