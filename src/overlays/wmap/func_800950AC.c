#include "common.h"

extern void func_800950EC(void);
extern s32 D_800DCEA8;
extern s32 D_801B2B64;
extern s32 D_801B2B60;

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800950AC(void)
{
    D_800DCEA8 = 0;
    D_801B2B64 = 0x10;
    D_801B2B60 += 1;
    func_800950EC();
}
