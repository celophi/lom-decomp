#include "common.h"

extern s32 D_801ADAE0;
extern s32 D_801B29B0;
extern s32 D_801B29B4;

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_8008B628(void)
{
    D_801ADAE0 = 1;
    D_801B29B4 = 0xC;
    D_801B29B0 += 1;
}
