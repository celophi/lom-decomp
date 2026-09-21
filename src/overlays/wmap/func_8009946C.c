#include "common.h"

extern void func_800994B8(void);
extern s16 D_800D944C[];
extern s32 D_801B2C3C;
extern s32 D_801B2C38;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8009946C(void)
{
    D_800D944C[17] = 0;
    D_800D944C[19] = 2;
    D_801B2C3C = 0x40;
    D_801B2C38 += 1;
    func_800994B8();
}
