#include "common.h"

extern s32 D_801398D0;
extern s32 D_801B3220;
extern void func_800C0294(void);

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800C0254(void)
{
    if (D_801398D0 != 2)
    {
        D_801B3220 += 1;
        func_800C0294();
    }
}
