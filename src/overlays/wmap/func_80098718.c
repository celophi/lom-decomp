#include "common.h"

extern void func_80098764(void);
extern s16 D_800D93F4[];
extern s32 D_801B2BFC;
extern s32 D_801B2BF8;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80098718(void)
{
    D_800D93F4[17] = 0;
    D_800D93F4[19] = 2;
    D_801B2BFC = 0x40;
    D_801B2BF8 += 1;
    func_80098764();
}
