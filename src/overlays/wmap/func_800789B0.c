#include "common.h"

extern void func_800789FC(void);
extern s16 D_800D9370[];
extern s32 D_801B2634;
extern s32 D_801B2630;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800789B0(void)
{
    D_800D9370[19] = 8;
    D_800D9370[17] = 0;
    D_801B2634 = 0x10;
    D_801B2630 += 1;
    func_800789FC();
}
