#include "common.h"

extern void func_80082CDC(void);
extern s16 D_800D939C[];
extern s32 D_801B2824;
extern s32 D_801B2820;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80082C90(void)
{
    D_800D939C[19] = 2;
    D_800D939C[17] = 0;
    D_801B2824 = 0x8;
    D_801B2820 += 1;
    func_80082CDC();
}
