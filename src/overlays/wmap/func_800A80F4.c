#include "common.h"

extern s32 D_801B2E54;
extern s32 D_801B2E50;
extern s32 D_80182DF8;
extern void func_800A8164(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A80F4(void)
{
    if (--D_801B2E54 == 0)
    {
        D_801B2E50 += 1;
    }
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A8128(void)
{
    if (D_80182DF8 == 0)
    {
        D_801B2E50 += 1;
        func_800A8164();
    }
}
