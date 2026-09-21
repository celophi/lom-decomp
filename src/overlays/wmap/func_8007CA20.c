#include "common.h"

extern void func_8007CA6C(void);
extern s16 D_800D93C8[];
extern s32 D_801B271C;
extern s32 D_801B2718;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8007CA20(void)
{
    D_800D93C8[19] = 8;
    D_800D93C8[17] = 0;
    D_801B271C = 0x10;
    D_801B2718 += 1;
    func_8007CA6C();
}
