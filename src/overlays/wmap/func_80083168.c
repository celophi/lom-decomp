#include "common.h"

extern void func_800831B4(void);
extern s16 D_800D93F4[];
extern s32 D_801B2834;
extern s32 D_801B2830;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80083168(void)
{
    D_800D93F4[19] = 8;
    D_800D93F4[17] = 0;
    D_801B2834 = 0x8;
    D_801B2830 += 1;
    func_800831B4();
}
