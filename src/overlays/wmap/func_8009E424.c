#include "common.h"

extern s32 D_801B2CD8;
extern s32 D_801B2CDC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009E424(void)
{
    D_801B2CD8 = 1;
    D_801B2CDC = 1;
}
