#include "common.h"

extern s32 D_801B2F30;
extern s32 D_801B2F34;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AFF98(void)
{
    D_801B2F30 = 1;
    D_801B2F34 = 1;
}
