#include "common.h"

extern s32 D_801B27A8;
extern s32 D_801B27AC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007FF60(void)
{
    D_801B27A8 = 1;
    D_801B27AC = 1;
}
