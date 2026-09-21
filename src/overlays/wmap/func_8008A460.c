#include "common.h"

extern void func_8008A4AC(void);
extern s16 D_800D9268[];
extern s32 D_801B2994;
extern s32 D_801B2990;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8008A460(void)
{
    D_800D9268[151] = 8;
    D_800D9268[149] = 0;
    D_801B2994 = 0x10;
    D_801B2990 += 1;
    func_8008A4AC();
}
