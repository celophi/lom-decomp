#include "common.h"

extern s32 D_801B2CC8;
extern s32 D_801B2CCC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009D4F4(void)
{
    D_801B2CC8 = 1;
    D_801B2CCC = 1;
}
