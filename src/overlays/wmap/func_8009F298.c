#include "common.h"

extern s32 D_801B2D00;
extern s32 D_801B2D04;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009F298(void)
{
    D_801B2D00 = 1;
    D_801B2D04 = 1;
}
