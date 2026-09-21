#include "common.h"

extern s32 D_801B25B0;
extern s32 D_801B25B4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007637C(void)
{
    D_801B25B0 = 1;
    D_801B25B4 = 1;
}
