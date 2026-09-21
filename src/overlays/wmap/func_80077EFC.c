#include "common.h"

extern s32 D_801B2608;
extern s32 D_801B260C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80077EFC(void)
{
    D_801B2608 = 1;
    D_801B260C = 1;
}
