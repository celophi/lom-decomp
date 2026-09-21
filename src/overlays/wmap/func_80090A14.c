#include "common.h"

extern void func_80090A60(void);
extern s16 D_800D9344[];
extern s32 D_801B2A8C;
extern s32 D_801B2A88;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80090A14(void)
{
    D_800D9344[19] = 8;
    D_800D9344[17] = 0;
    D_801B2A8C = 0x10;
    D_801B2A88 += 1;
    func_80090A60();
}
