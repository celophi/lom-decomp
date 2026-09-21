#include "common.h"

extern s32 D_801B24F8;
extern s32 D_801B24FC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80072304(void)
{
    D_801B24F8 = 1;
    D_801B24FC = 1;
}
