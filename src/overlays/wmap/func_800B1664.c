#include "common.h"

extern void func_800B16B0(void);
extern s16 D_800D93C8[];
extern s32 D_801B2F8C;
extern s32 D_801B2F88;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800B1664(void)
{
    D_800D93C8[19] = 4;
    D_800D93C8[17] = 0;
    D_801B2F8C = 0x20;
    D_801B2F88 += 1;
    func_800B16B0();
}
