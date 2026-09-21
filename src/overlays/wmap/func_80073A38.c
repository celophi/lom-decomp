#include "common.h"

extern s32 D_801B2530;
extern s32 D_801B2534;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80073A38(void)
{
    D_801B2530 = 1;
    D_801B2534 = 1;
}
