#include "common.h"

extern void func_80092E2C(void);
extern s32 D_801B2B14;
extern s32 D_801B2B10;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80092DF4(void)
{
    D_801B2B14 = 0x60;
    D_801B2B10 += 1;
    func_80092E2C();
}
