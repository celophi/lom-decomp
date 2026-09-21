#include "common.h"

extern void func_80086EB4(void);
extern s16 D_800D93C8[];
extern s32 D_801B2904;
extern s32 D_801B2900;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80086E68(void)
{
    D_800D93C8[19] = 16;
    D_800D93C8[17] = 0;
    D_801B2904 = 0x8;
    D_801B2900 += 1;
    func_80086EB4();
}
