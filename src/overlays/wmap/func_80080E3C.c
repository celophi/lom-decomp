#include "common.h"

extern s32 D_801B27E0;
extern s32 D_801B27E4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80080E3C(void)
{
    D_801B27E0 = 1;
    D_801B27E4 = 1;
}
