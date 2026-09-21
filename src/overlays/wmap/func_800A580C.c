#include "common.h"

extern void func_800A584C(void);
extern s32 D_800DCEB0;
extern s32 D_801B2E2C;
extern s32 D_801B2E28;

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A580C(void)
{
    D_800DCEB0 = 0;
    D_801B2E2C = 0xA0;
    D_801B2E28 += 1;
    func_800A584C();
}
