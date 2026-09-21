#include "common.h"

extern s32 D_801B2918;
extern s32 D_801B291C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80087C08(void)
{
    D_801B2918 = 1;
    D_801B291C = 1;
}
