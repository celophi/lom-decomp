#include "common.h"

extern s32 D_801B2568;
extern s32 D_801B256C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007502C(void)
{
    D_801B2568 = 1;
    D_801B256C = 1;
}
