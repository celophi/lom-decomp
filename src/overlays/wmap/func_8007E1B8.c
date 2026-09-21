#include "common.h"

extern s32 D_801B2750;
extern s32 D_801B2754;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007E1B8(void)
{
    D_801B2750 = 1;
    D_801B2754 = 1;
}
