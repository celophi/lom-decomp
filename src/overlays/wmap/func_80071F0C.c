#include "common.h"

extern s32 D_801B24E0;
extern s32 D_801B24E4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80071F0C(void)
{
    D_801B24E0 = 1;
    D_801B24E4 = 1;
}
