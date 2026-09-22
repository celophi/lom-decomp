#include "common.h"

extern s32 D_801B2E4C;
extern s32 D_801B2E48;
extern s32 D_80182DF8;
extern void func_800A7D7C(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A7D0C(void)
{
    if (--D_801B2E4C == 0)
    {
        D_801B2E48 += 1;
    }
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A7D40(void)
{
    if (D_80182DF8 == 0)
    {
        D_801B2E48 += 1;
        func_800A7D7C();
    }
}
