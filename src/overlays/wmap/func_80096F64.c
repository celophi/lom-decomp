#include "common.h"

extern void func_80096FA4(void);
extern s32 D_800DCEA8;
extern s32 D_801B2BC4;
extern s32 D_801B2BC0;

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_80096F64(void)
{
    D_800DCEA8 = 0;
    D_801B2BC4 = 0x14;
    D_801B2BC0 += 1;
    func_80096FA4();
}
