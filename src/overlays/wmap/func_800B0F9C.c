#include "common.h"

extern s32 D_801B2F78;
extern s32 D_801B2F7C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B0F9C(void)
{
    D_801B2F78 = 1;
    D_801B2F7C = 1;
}
