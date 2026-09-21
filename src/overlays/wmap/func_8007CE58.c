#include "common.h"

extern void func_8007CEA4(void);
extern s16 D_800D93F4[];
extern s32 D_801B272C;
extern s32 D_801B2728;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8007CE58(void)
{
    D_800D93F4[19] = 8;
    D_800D93F4[17] = 0;
    D_801B272C = 0x10;
    D_801B2728 += 1;
    func_8007CEA4();
}
