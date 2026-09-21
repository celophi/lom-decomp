#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B25E8;
extern void func_80077670(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80077634(void)
{
    if (D_8013B20C == 0)
    {
        D_801B25E8 += 1;
        func_80077670();
    }
}
