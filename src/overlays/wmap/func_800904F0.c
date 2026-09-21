#include "common.h"

extern s32 D_801B2A78;
extern s32 D_801B2A7C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800904F0(void)
{
    D_801B2A78 = 1;
    D_801B2A7C = 1;
}
