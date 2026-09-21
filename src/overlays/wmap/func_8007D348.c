#include "common.h"

extern void func_8007D394(void);
extern s16 D_800D9BB0[];
extern s32 D_801B273C;
extern s32 D_801B2738;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8007D348(void)
{
    D_800D9BB0[19] = 8;
    D_800D9BB0[17] = 0;
    D_801B273C = 0x10;
    D_801B2738 += 1;
    func_8007D394();
}
