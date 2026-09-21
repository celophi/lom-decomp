#include "common.h"

extern void func_800A5504(void);
extern s32 D_800DCEAC;
extern s32 D_801B2E24;
extern s32 D_801B2E20;

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A54C4(void)
{
    D_800DCEAC = 0;
    D_801B2E24 = 0x18;
    D_801B2E20 += 1;
    func_800A5504();
}
