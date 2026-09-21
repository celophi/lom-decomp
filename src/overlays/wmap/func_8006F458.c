#include "common.h"

extern s32 D_801B2428;
extern s32 D_801B242C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006F458(void)
{
    D_801B2428 = 1;
    D_801B242C = 1;
}
