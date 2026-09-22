#include "common.h"

extern void func_800A02EC(void);
extern s16 D_800D93C8[];
extern s32 D_801B2D3C;
extern s32 D_801B2D38;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800A02A0(void)
{
    D_800D93C8[19] = 4;
    D_800D93C8[17] = 0;
    D_801B2D3C = 0x20;
    D_801B2D38 += 1;
    func_800A02EC();
}
