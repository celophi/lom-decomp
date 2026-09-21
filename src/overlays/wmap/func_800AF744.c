#include "common.h"

extern void func_800AF790(void);
extern s16 D_800D9318[];
extern s32 D_801B2F04;
extern s32 D_801B2F00;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800AF744(void)
{
    D_800D9318[19] = 4;
    D_800D9318[17] = 0;
    D_801B2F04 = 0x20;
    D_801B2F00 += 1;
    func_800AF790();
}
