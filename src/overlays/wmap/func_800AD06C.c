#include "common.h"

extern s32 D_801B2EB8;
extern s32 D_801B2EBC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AD06C(void)
{
    D_801B2EB8 = 1;
    D_801B2EBC = 1;
}
