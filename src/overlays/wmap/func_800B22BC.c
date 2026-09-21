#include "common.h"

extern s32 D_801B2F98;
extern s32 D_801B2F9C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B22BC(void)
{
    D_801B2F98 = 1;
    D_801B2F9C = 1;
}
