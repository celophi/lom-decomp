#include "common.h"

extern s32 D_801B2E60;
extern s32 D_801B2E64;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A884C(void)
{
    D_801B2E60 = 1;
    D_801B2E64 = 1;
}
