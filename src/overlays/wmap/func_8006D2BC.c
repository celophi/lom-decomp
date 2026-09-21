#include "common.h"

extern s32 D_801B10A0;
extern s32 D_801B10A4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006D2BC(void)
{
    D_801B10A0 = 1;
    D_801B10A4 = 1;
}
