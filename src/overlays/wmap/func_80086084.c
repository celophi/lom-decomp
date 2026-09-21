#include "common.h"

extern s32 D_801B28C0;
extern s32 D_801B28C4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80086084(void)
{
    D_801B28C0 = 1;
    D_801B28C4 = 1;
}
