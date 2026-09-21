#include "common.h"

extern s32 D_801B2AF8;
extern s32 D_801B2AFC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80092910(void)
{
    D_801B2AF8 = 1;
    D_801B2AFC = 1;
}
