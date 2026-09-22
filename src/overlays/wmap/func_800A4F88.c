#include "common.h"

extern void func_800A4FC8(void);
extern s32 D_800DCEA8;
extern s32 D_801B2E14;
extern s32 D_801B2E10;

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A4F88(void)
{
    D_800DCEA8 = 0;
    D_801B2E14 = 0x14;
    D_801B2E10 += 1;
    func_800A4FC8();
}
