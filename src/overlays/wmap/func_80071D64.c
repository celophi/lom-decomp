#include "common.h"

extern s32 D_801B24D8;
extern s32 D_801B24DC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80071D64(void)
{
    D_801B24D8 = 1;
    D_801B24DC = 1;
}
