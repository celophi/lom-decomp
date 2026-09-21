#include "common.h"

extern s32 D_801B2AE0;
extern s32 D_801B2AE4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009236C(void)
{
    D_801B2AE0 = 1;
    D_801B2AE4 = 1;
}
