#include "common.h"

extern void func_80092F1C(void);
extern s32 D_800DCEA8;
extern s32 D_801B2B14;
extern s32 D_801B2B10;

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_80092EDC(void)
{
    D_800DCEA8 = 0;
    D_801B2B14 = 0x40;
    D_801B2B10 += 1;
    func_80092F1C();
}
