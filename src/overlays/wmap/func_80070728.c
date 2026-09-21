#include "common.h"

extern s32 D_801B2408;
extern s32 D_801B240C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80070728(void)
{
    D_801B2408 = 1;
    D_801B240C = 1;
}
