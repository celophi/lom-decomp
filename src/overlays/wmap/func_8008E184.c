#include "common.h"

extern s32 D_801B2A30;
extern s32 D_801B2A34;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008E184(void)
{
    D_801B2A30 = 1;
    D_801B2A34 = 1;
}
