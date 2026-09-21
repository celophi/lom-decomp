#include "common.h"

extern s32 D_801B2D20;
extern s32 D_801B2D24;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009F958(void)
{
    D_801B2D20 = 1;
    D_801B2D24 = 1;
}
