#include "common.h"

extern s32 D_801B2958;
extern s32 D_801B295C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800891AC(void)
{
    D_801B2958 = 1;
    D_801B295C = 1;
}
