#include "common.h"

extern s32 D_801B2B78;
extern s32 D_801B2B7C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800955A4(void)
{
    D_801B2B78 = 1;
    D_801B2B7C = 1;
}
