#include "common.h"

extern s32 D_801B2B00;
extern s32 D_801B2B04;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80092A68(void)
{
    D_801B2B00 = 1;
    D_801B2B04 = 1;
}
