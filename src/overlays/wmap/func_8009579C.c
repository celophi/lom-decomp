#include "common.h"

extern s32 D_801B2B80;
extern s32 D_801B2B84;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009579C(void)
{
    D_801B2B80 = 1;
    D_801B2B84 = 1;
}
