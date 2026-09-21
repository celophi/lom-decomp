#include "common.h"

extern s32 D_801B2A98;
extern s32 D_801B2A9C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80090D64(void)
{
    D_801B2A98 = 1;
    D_801B2A9C = 1;
}
