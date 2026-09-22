#include "common.h"

extern s32 D_801ADAE0;
extern s32 D_801B26E8;
extern s32 D_801B26EC;

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_8007BD50(void)
{
    D_801ADAE0 = 1;
    D_801B26EC = 0x46;
    D_801B26E8 += 1;
}
