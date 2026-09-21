#include "common.h"

extern void func_8009CBD0(void);
extern s16 D_800D9370[];
extern s32 D_801B2CA4;
extern s32 D_801B2CA0;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8009CB84(void)
{
    D_800D9370[19] = 4;
    D_800D9370[17] = 0;
    D_801B2CA4 = 0x80;
    D_801B2CA0 += 1;
    func_8009CBD0();
}
