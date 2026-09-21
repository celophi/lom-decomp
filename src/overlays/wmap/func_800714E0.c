#include "common.h"

extern s32 D_801B24C0;
extern s32 D_801B24C4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800714E0(void)
{
    D_801B24C0 = 1;
    D_801B24C4 = 1;
}
