#include "common.h"

extern s32 D_801B1098;
extern s32 D_801B109C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006C894(void)
{
    D_801B1098 = 1;
    D_801B109C = 1;
}
