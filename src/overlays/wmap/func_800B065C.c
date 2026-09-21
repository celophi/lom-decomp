#include "common.h"

extern void func_800B06A8(void);
extern s16 D_800D9370[];
extern s32 D_801B2F54;
extern s32 D_801B2F50;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800B065C(void)
{
    D_800D9370[19] = 2;
    D_800D9370[17] = 0;
    D_801B2F54 = 0x40;
    D_801B2F50 += 1;
    func_800B06A8();
}
