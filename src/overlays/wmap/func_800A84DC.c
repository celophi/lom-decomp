#include "common.h"

extern s32 D_801B2E5C;
extern s32 D_801B2E58;
extern s32 D_80182DF8;
extern void func_800A854C(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A84DC(void)
{
    if (--D_801B2E5C == 0)
    {
        D_801B2E58 += 1;
    }
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A8510(void)
{
    if (D_80182DF8 == 0)
    {
        D_801B2E58 += 1;
        func_800A854C();
    }
}
