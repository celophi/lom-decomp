#include "common.h"

extern s32 D_801B2A18;
extern s32 D_801B2A1C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008DC74(void)
{
    D_801B2A18 = 1;
    D_801B2A1C = 1;
}
