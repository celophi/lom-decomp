#include "common.h"

extern void func_80082F48(void);
extern s16 D_800D93C8[];
extern s32 D_801B282C;
extern s32 D_801B2828;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80082EFC(void)
{
    D_800D93C8[19] = 8;
    D_800D93C8[17] = 0;
    D_801B282C = 0x8;
    D_801B2828 += 1;
    func_80082F48();
}
