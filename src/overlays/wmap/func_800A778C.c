#include "common.h"

extern void func_800A77CC(void);
extern s32 D_800DCEC0;
extern s32 D_801B2E44;
extern s32 D_801B2E40;

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A778C(void)
{
    D_800DCEC0 = 0;
    D_801B2E44 = 0x1E;
    D_801B2E40 += 1;
    func_800A77CC();
}
