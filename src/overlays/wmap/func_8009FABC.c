#include "common.h"

extern void func_8009FB08(void);
extern s16 D_800D9370[];
extern s32 D_801B2D24;
extern s32 D_801B2D20;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8009FABC(void)
{
    D_800D9370[19] = 2;
    D_800D9370[17] = 0;
    D_801B2D24 = 0x40;
    D_801B2D20 += 1;
    func_8009FB08();
}
