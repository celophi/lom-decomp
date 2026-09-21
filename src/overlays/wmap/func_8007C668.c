#include "common.h"

extern s32 D_801B2708;
extern s32 D_801B270C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007C668(void)
{
    D_801B2708 = 1;
    D_801B270C = 1;
}
