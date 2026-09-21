#include "common.h"

extern void func_8009484C(void);
extern s16 D_800D9318[];
extern s32 D_801B2B3C;
extern s32 D_801B2B38;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80094800(void)
{
    D_800D9318[17] = 0;
    D_800D9318[19] = 8;
    D_801B2B3C = 0x10;
    D_801B2B38 += 1;
    func_8009484C();
}
