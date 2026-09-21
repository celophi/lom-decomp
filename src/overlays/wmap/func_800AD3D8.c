#include "common.h"

extern s32 D_801B2ED0;
extern s32 D_801B2ED4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AD3D8(void)
{
    D_801B2ED0 = 1;
    D_801B2ED4 = 1;
}
