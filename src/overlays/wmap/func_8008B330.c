#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B29A8;
extern void func_8008B36C(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8008B330(void)
{
    if (D_8013B20C == 0)
    {
        D_801B29A8 += 1;
        func_8008B36C();
    }
}
