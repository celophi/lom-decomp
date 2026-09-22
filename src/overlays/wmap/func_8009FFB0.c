#include "common.h"

extern void func_8009FFFC(void);
extern s16 D_800D939C[];
extern s32 D_801B2D34;
extern s32 D_801B2D30;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8009FFB0(void)
{
    D_800D939C[19] = 4;
    D_800D939C[17] = 0;
    D_801B2D34 = 0x20;
    D_801B2D30 += 1;
    func_8009FFFC();
}
