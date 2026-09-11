#include "common.h"

extern s32 D_8010AE58;
extern s32 D_8010AE6C;
extern s32 D_8010AE70;

/**
 * @brief Reset the interpolation lower bound and seed the upper bound from the field bounds block.
 */
void func_800920FC(void)
{
    D_8010AE6C = 0;
    D_8010AE58 = 0x20;
    D_8010AE70 = *(s16*)0x801ED400;
}
