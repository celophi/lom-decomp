#include "common.h"

extern s32 D_801B25A8;
extern s32 D_801B25AC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80076230(void)
{
    D_801B25A8 = 1;
    D_801B25AC = 1;
}
