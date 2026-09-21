#include "common.h"

extern s32 D_801B2BD8;
extern s32 D_801B2BDC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800974A4(void)
{
    D_801B2BD8 = 1;
    D_801B2BDC = 1;
}
