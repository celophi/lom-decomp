#include "common.h"

extern void func_800B140C(void);
extern s32 D_800DCEAC;
extern s32 D_801B2F84;
extern s32 D_801B2F80;

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800B13CC(void)
{
    D_800DCEAC = 0;
    D_801B2F84 = 0x64;
    D_801B2F80 += 1;
    func_800B140C();
}
