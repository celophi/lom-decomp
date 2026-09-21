#include "common.h"

extern void func_80083BF4(void);
extern s32 D_800DCEA8;
extern s32 D_801B285C;
extern s32 D_801B2858;

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_80083BB4(void)
{
    D_800DCEA8 = 0;
    D_801B285C = 0x18;
    D_801B2858 += 1;
    func_80083BF4();
}
