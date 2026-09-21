#include "common.h"

extern void func_800A0630(void);
extern s32 D_800DCEAC;
extern s32 D_801B2D44;
extern s32 D_801B2D40;

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A05F0(void)
{
    D_800DCEAC = 0;
    D_801B2D44 = 0x40;
    D_801B2D40 += 1;
    func_800A0630();
}
