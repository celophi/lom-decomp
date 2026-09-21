#include "common.h"

extern void func_800907F4(void);
extern s16 D_800D93C8[];
extern s32 D_801B2A84;
extern s32 D_801B2A80;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800907A8(void)
{
    D_800D93C8[19] = 4;
    D_800D93C8[17] = 0;
    D_801B2A84 = 0x20;
    D_801B2A80 += 1;
    func_800907F4();
}
