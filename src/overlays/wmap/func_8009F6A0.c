#include "common.h"

extern s32 D_801B2D10;
extern s32 D_801B2D14;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009F6A0(void)
{
    D_801B2D10 = 1;
    D_801B2D14 = 1;
}
