#include "common.h"

extern void func_800933B4(void);
extern s16 D_800D93F4[];
extern s32 D_801B2B24;
extern s32 D_801B2B20;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80093368(void)
{
    D_800D93F4[19] = 4;
    D_800D93F4[17] = 0;
    D_801B2B24 = 0x20;
    D_801B2B20 += 1;
    func_800933B4();
}
