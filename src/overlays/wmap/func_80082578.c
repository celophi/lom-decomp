#include "common.h"

extern s32 D_801B2800;
extern s32 D_801B2804;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80082578(void)
{
    D_801B2800 = 1;
    D_801B2804 = 1;
}
