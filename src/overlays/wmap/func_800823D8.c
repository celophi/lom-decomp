#include "common.h"

extern s32 D_801B27F8;
extern s32 D_801B27FC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800823D8(void)
{
    D_801B27F8 = 1;
    D_801B27FC = 1;
}
