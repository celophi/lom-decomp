#include "common.h"

extern s32 D_801B2840;
extern s32 D_801B2844;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80083530(void)
{
    D_801B2840 = 1;
    D_801B2844 = 1;
}
