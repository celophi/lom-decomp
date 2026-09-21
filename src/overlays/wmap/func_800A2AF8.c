#include "common.h"

extern s32 D_801B2DA8;
extern s32 D_801B2DAC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A2AF8(void)
{
    D_801B2DA8 = 1;
    D_801B2DAC = 1;
}
