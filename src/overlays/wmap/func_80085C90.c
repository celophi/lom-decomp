#include "common.h"

extern s32 D_801B28B8;
extern s32 D_801B28BC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80085C90(void)
{
    D_801B28B8 = 1;
    D_801B28BC = 1;
}
