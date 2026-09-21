#include "common.h"

extern void func_8007C564(void);
extern s16 D_800D9370[];
extern s32 D_801B2704;
extern s32 D_801B2700;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8007C518(void)
{
    D_800D9370[19] = 8;
    D_800D9370[17] = 0;
    D_801B2704 = 0x40;
    D_801B2700 += 1;
    func_8007C564();
}
